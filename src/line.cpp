#include "topicCorpus.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <gsl/gsl_rng.h>
using namespace std;

const int sigmoid_table_size = 1000;
long long num_vertices_l = 0;
int threads, negatives, dims, isb;
float irho, rho;

const int neg_table_size = 1e8;
int m_n_vertices_l = max_num_vertices;

int *neg_table;
float *sigmoid_table;
float *emb_context;


/* Fastly compute sigmoid function */
void InitSigmoidTable()
{
	float x;
	sigmoid_table = (float *)malloc((sigmoid_table_size + 1) * sizeof(float));
	for (int k = 0; k != sigmoid_table_size; k++)
	{
		x = 2 * SIGMOID_BOUND * k / sigmoid_table_size - SIGMOID_BOUND;
		sigmoid_table[k] = 1 / (1 + exp(-x));
	}
}

float FastSigmoid(float x)
{
	if (x > SIGMOID_BOUND) return 1;
	else if (x < -SIGMOID_BOUND) return 0;
	int k = (x + SIGMOID_BOUND) * sigmoid_table_size / SIGMOID_BOUND / 2;
	return sigmoid_table[k];
}

long long total_samples, current_sample_count = 0, num_tedges = 0;
float *emb_vertex;
int *edge_source_id, *edge_target_id;
double *edge_weight;

// Parameters for edge sampling
long long *alias;
double *prob;

const gsl_rng_type * gsl_T;
gsl_rng * gsl_r;

std::map<int, std::string>::iterator aviter_l;

/* Add a vertex to the vertex set */
void topicCorpus::AddVertex_l(int vid)
{
    printf("AddVertex_l vid:%d \n",vid);fflush(stdout);
    aviter_l = corp->rItemIds.find(vid);
    v = new ClassVertex();
    v -> name = aviter_l -> second;
    V.insert(make_pair(vid,v));

    num_vertices_l++;
}

/* Read network from the training file */
void topicCorpus::ReadData_l()
{
    	int vid;

    	fstream fin;
	string str;
	int name_v1, name_v2;
	std::map<int, ClassVertex*>::iterator iter;

	double weight;

    // read train file
    fin.open("ReconstructedTrainFile.txt");
    if (fin.is_open())
    {
        cout << "Reconstructed Train file successfully opened. (1)\n";
    }
    else
    {
        cout << "ERROR: Reconstructed Train file not found!\n";
        exit(1);
    }

	num_tedges = 0;
	while (getline (fin, str)) num_tedges++;
	fin.close();
	printf("Number of train edges: %lld          \n", num_tedges);

	edge_source_id = (int *)malloc(num_tedges*sizeof(int));
	edge_target_id = (int *)malloc(num_tedges*sizeof(int));
	edge_weight = (double *)malloc(num_tedges*sizeof(double));
	if (edge_source_id == NULL || edge_target_id == NULL || edge_weight == NULL)
	{
		printf("Error: (t) memory allocation failed!\n");
		exit(1);
	}

	fin.open("ReconstructedTrainFile.txt");
	num_vertices_l = 0;
	for (int k = 0; k != num_tedges; k++)
	{
		getline (fin, str);
	    stringstream ss(str);
        ss >> name_v1 >> name_v2 >> weight;
        printf("%d %d %lf\n", name_v1, name_v2, weight);fflush(stdout);

		if (k % 10000 == 0)
		{
			printf("Reading train edges: %.3lf%%%c", k / (double)(num_tedges + 1) * 100, 13);
			fflush(stdout);
		}

		iter = V.find(name_v1);
		if (iter == V.end())
        {
            AddVertex_l(name_v1);
        }
        V[name_v1]->degree += weight;
        edge_source_id[k] = name_v1;

		iter = V.find(name_v2);
		if (iter == V.end())
        {
            AddVertex_l(name_v2);
        }
		V[name_v2]->degree += weight;
		edge_target_id[k] = name_v2;
		edge_weight[k] = weight;
	}
	fin.close();
	printf("Number of train vertices: %d          \n", num_vertices_l);
}

/* The alias sampling algorithm, which is used to sample an edge in O(1) time. */
void InitAliasTable()
{
	alias = (long long *)malloc(num_tedges*sizeof(long long));
	prob = (double *)malloc(num_tedges*sizeof(double));
	if (alias == NULL || prob == NULL)
	{
		printf("Error: memory allocation failed!\n");
		exit(1);
	}

	double *norm_prob = (double*)malloc(num_tedges*sizeof(double));
	long long *large_block = (long long*)malloc(num_tedges*sizeof(long long));
	long long *small_block = (long long*)malloc(num_tedges*sizeof(long long));
	if (norm_prob == NULL || large_block == NULL || small_block == NULL)
	{
		printf("Error: memory allocation failed!\n");
		exit(1);
	}

	double sum = 0;
	long long cur_small_block, cur_large_block;
	long long num_small_block = 0, num_large_block = 0;

	for (long long k = 0; k != num_tedges; k++) sum += edge_weight[k];
	for (long long k = 0; k != num_tedges; k++) norm_prob[k] = edge_weight[k] * num_tedges / sum;

	for (long long k = num_tedges - 1; k >= 0; k--)
	{
		if (norm_prob[k]<1)
			small_block[num_small_block++] = k;
		else
			large_block[num_large_block++] = k;
	}

	while (num_small_block && num_large_block)
	{
		cur_small_block = small_block[--num_small_block];
		cur_large_block = large_block[--num_large_block];
		prob[cur_small_block] = norm_prob[cur_small_block];
		alias[cur_small_block] = cur_large_block;
		norm_prob[cur_large_block] = norm_prob[cur_large_block] + norm_prob[cur_small_block] - 1;
		if (norm_prob[cur_large_block] < 1)
			small_block[num_small_block++] = cur_large_block;
		else
			large_block[num_large_block++] = cur_large_block;
	}

	while (num_large_block) prob[large_block[--num_large_block]] = 1;
	while (num_small_block) prob[small_block[--num_small_block]] = 1;

	free(norm_prob);
	free(small_block);
	free(large_block);
}

long long SampleAnEdge(double rand_value1, double rand_value2)
{
	long long k = (long long)num_tedges * rand_value1;
	return rand_value2 < prob[k] ? k : alias[k];
}

/* Initialize the vertex embedding and the context embedding */
void topicCorpus::InitVector()
{
	long long a, b;

	a = posix_memalign((void **)&emb_vertex, 128, (long long)nItems * dims * sizeof(float));
	if (emb_vertex == NULL) { printf("Error: memory allocation failed\n"); exit(1); }
	for (b = 0; b < dims; b++) for (a = 0; a < nItems; a++)
		emb_vertex[a * dims + b] = (rand() / (float)RAND_MAX - 0.5) / dims;

	a = posix_memalign((void **)&emb_context, 128, (long long)nItems * dims * sizeof(float));
	if (emb_context == NULL) { printf("Error: memory allocation failed\n"); exit(1); }
	for (b = 0; b < dims; b++) for (a = 0; a < nItems; a++)
		emb_context[a * dims + b] = 0;
}

/* Sample negative vertex samples according to vertex degrees */ // Inverse Transform Method
void topicCorpus::InitNegTable()
{
	double sum = 0, cur_sum = 0, por = 0;
	int vid = 0, k = 0;
	neg_table = (int *)malloc(neg_table_size * sizeof(int));
	std::map<std::string, int>::iterator initer;

	for (std::map<int, ClassVertex*>::iterator it = V.begin(); it != V.end(); it ++)
	{
	    initer = corp->itemIds.find(it->second->name);
        k = initer->second;
        sum += pow(V[k]->degree, NEG_SAMPLING_POWER);// pow(x,y)=x^y
	}

    std::map<int, ClassVertex*>::iterator it = V.begin();

	    for (int k = 0; k != neg_table_size && it != V.end(); k++)
        {
            if ((double)(k + 1) / neg_table_size > por)
            {
                cur_sum += pow( V[vid]->degree, NEG_SAMPLING_POWER);
                por = cur_sum / sum;
                it ++;
                initer = corp->itemIds.find(it->second->name);
                vid = initer->second;
            }
            neg_table[k] = vid - 1;
        }
}

/* Fastly generate a random integer */
int Rand(unsigned long long &seed)
{
	seed = seed * 25214903917 + 11;
	return (seed >> 16) % neg_table_size;
}

/* Update embeddings */
void Update(float *vec_u, float *vec_v, float *vec_error, int label)
{
	float x = 0, g;
	for (int c = 0; c != dims; c++) x += vec_u[c] * vec_v[c];
	g = (label - FastSigmoid(x)) * rho;
	for (int c = 0; c != dims; c++) vec_error[c] += g * vec_v[c];
	for (int c = 0; c != dims; c++) vec_v[c] += g * vec_u[c];
}

void *TrainLINEThread(void *id)
{
	long long u, v, lu, lv, target, label;
	long long count = 0, last_count = 0, curedge;
	unsigned long long seed = (long long)id;
	float *vec_error = (float *)calloc(dims, sizeof(float));

	while (1)
	{
		//judge for exit: use up all samples
		if (count > total_samples / threads + 2) break;

		if (count - last_count>10000)
		{
			current_sample_count += count - last_count;
			last_count = count;
			printf("%cRho: %f  Progress: %.3lf%%", 13, rho, (float)current_sample_count / (float)(total_samples + 1) * 100);
			fflush(stdout);
			rho = irho * (1 - current_sample_count / (float)(total_samples + 1));
			if (rho < irho * 0.0001) rho = irho * 0.0001;
		}

		curedge = SampleAnEdge(gsl_rng_uniform(gsl_r), gsl_rng_uniform(gsl_r));
		u = edge_source_id[curedge];
		v = edge_target_id[curedge];

		lu = u * dims;
		for (int c = 0; c != dims; c++) vec_error[c] = 0;

		// NEGATIVE SAMPLING
		for (int d = 0; d != negatives + 1; d++)
		{
			if (d == 0)
			{
				target = v;
				label = 1;
			}
			else
			{
				target = neg_table[Rand(seed)];
				label = 0;
			}
			lv = target * dims;

                Update(&emb_vertex[lu], &emb_context[lv], vec_error, label);
		}
		for (int c = 0; c != dims; c++) emb_vertex[c + lu] += vec_error[c];

		count++;
	}
	free(vec_error);
	pthread_exit(NULL);
}

void topicCorpus::Output_l()
{
    FILE *fo = fopen("embedding_file_2.txt", "wb");;
    if (fo == NULL)
	{
		printf("ERROR: Failed to open embedding file !\n");
		exit(1);
	}
	fprintf(fo, "%lld\t%d\n", num_vertices_l, dims);

    std::map<std::string, int>::iterator oliter;

	for (std::map<int, ClassVertex*>::iterator it = V.begin(); it != V.end(); it ++)
	{
	    oliter = corp->itemIds.find(it->second->name);
        int a = oliter->second;

		fprintf(fo, "%d\t", a);
		printf("a=%d\n",a);
		for (int b = 0; b < dims; b++) fprintf(fo, "%lf\t", emb_vertex[a * dims + b]);
		fprintf(fo, "\n");
	}
	fclose(fo);
}

void topicCorpus::TrainLINE() {
	long a;
	pthread_t *pt = (pthread_t *)malloc(threads * sizeof(pthread_t));

	ReadData_l();
	InitAliasTable();
	InitVector();
	InitNegTable();
	InitSigmoidTable();

	gsl_rng_env_setup();
	gsl_T = gsl_rng_rand48;
	gsl_r = gsl_rng_alloc(gsl_T);
	gsl_rng_set(gsl_r, 314159265);

	clock_t start = clock();
	printf("--------------------------------\n");
	for (a = 0; a < threads; a++) pthread_create(&pt[a], NULL, TrainLINEThread, (void *)a);
	for (a = 0; a < threads; a++) pthread_join(pt[a], NULL);
	printf("\n");
	clock_t finish = clock();
	printf("Total time: %lf\n", (double)(finish - start) / CLOCKS_PER_SEC);

	Output_l();
}

void topicCorpus::line(int total_train)
{
    total_samples = total_train;
	irho = init_rho;
	threads = num_threads;
	negatives = num_negative;
	dims = dim;
	isb = is_binary;
    TrainLINE();
}
