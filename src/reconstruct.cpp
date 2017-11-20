#include "topicCorpus.hpp"
#include "common.hpp"
#include "corpus.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <map>
#include <queue>
using namespace std;

struct Neighbor
{
	int vid;
	double weight;
	friend bool operator < (Neighbor n1, Neighbor n2)
	{
		return n1.weight > n2.weight;
	}
};

int num_vertices_r = 0;
long long num_edges_r = 0;

int max_depth = 1, max_k = 0;
std::vector<int> vertex_set;
std::vector<Neighbor> *neighbor;

Neighbor *rank_list;
std::map<int, double> vid2weight;

int m_n_vertices = max_num_vertices;

std::map<int, std::string>::iterator aviter_r;

/* Add a vertex to the vertex set */
void topicCorpus::AddVertex_r(int vid)
{
    printf("AddVertex_r vid:%d \n",vid);fflush(stdout);
    aviter_r = corp->rItemIds.find(vid);
    v_r = new ClassVertex_r();
    v_r -> name = aviter_r -> second;
    v_r -> sum_weight = 0;
    V_r.insert(make_pair(vid,v_r));

	num_vertices_r++;
}

/* Read network from the training file */
void topicCorpus::ReadData(char* trainFilePath)
{
    fstream fin;
    fin.open(trainFilePath);
    if (fin.is_open())
    {
        cout << "network file successfully opened. (1)\n";
    }
    else
    {
        cout << "ERROR: network file not found!\n";
        exit(1);
    }

	string str;
	int name_v1, name_v2, u, v;
	double weight;
	std::map<int, ClassVertex_r*>::iterator itervr;
	Neighbor nb;

	num_edges_r = 0;
	while (getline (fin, str)) num_edges_r++;
	fin.close();
	printf("Number of edges: %lld          \n", num_edges_r);

	fin.open(trainFilePath);
	if (fin.is_open())
    {
        cout << "network file successfully opened. (2)\n";
    }
    else
    {
        cout << "ERROR: network file not found!\n";
        exit(1);
    }
	num_vertices_r = 0;
	for (int k = 0; k != num_edges_r; k++)
	{
	    printf("^");
	    getline (fin, str);
	    stringstream ss(str);
        ss >> name_v1 >> name_v2 >> weight;
        printf("%d %d %lf\n", name_v1, name_v2, weight);fflush(stdout);

        printf("#");

        itervr = V_r.find(name_v1);
		if (itervr == V_r.end())
        {
            AddVertex_r(name_v1);
            V_r[name_v1]->degree += weight;
        }

		itervr = V_r.find(name_v2);
		if (itervr == V_r.end())
        {
            AddVertex_r(name_v2);
            V_r[name_v2]->degree += weight;
        }
	}
	fin.close();
	printf("Number of vertices: %d          \n", num_vertices_r);

	neighbor = new std::vector<Neighbor>[nItems];
	rank_list = (Neighbor *)calloc(nItems, sizeof(Neighbor));

	fin.open(trainFilePath);
	for (long long k = 0; k != num_edges_r; k++)
	{
		getline (fin, str);
	    stringstream ss(str);
        ss >> name_v1 >> name_v2 >> weight;

		if (k % 10 == 0)
		{
			printf("Reading neighbors: %.3lf%%%c", k / (double)(num_edges_r + 1) * 100, 13);
			fflush(stdout);
		}

		nb.vid = name_v2;
		nb.weight = weight;
		neighbor[name_v1].push_back(nb);
	}
	fin.close();
	printf("\n");

	std::map<std::string, int>::iterator niter;

	for (std::map<int, ClassVertex_r*>::iterator it = V_r.begin(); it != V_r.end(); it ++)
    {
        it->second->sum_weight = 0;
        niter = corp->itemIds.find(it->second->name);
        int id = niter->second;
		int len = neighbor[id].size();
		for (int i = 0; i != len; i++)
			it->second->sum_weight += neighbor[id][i].weight;
    }
}

void topicCorpus::createTVTFile(char* trainFilePath)
{
    int g=0;
    this->total_train=0;
    this->total_validation=0;
    this->total_test=0;

    FILE *fo1 = fopen(trainFilePath, "wb");
    FILE *fo4 = fopen("nonedge.txt", "wb");
    char pF[100],pT[100];
    if (fo1 == NULL)
	{
		printf("ERROR: Failed to create train file !\n");
		exit(1);
	}
    for (int i = 0; i < validStart; i ++)
    {
        edge* e = edges[i];

        if(not e->labels[g])
        {
            snprintf(pF, sizeof(pF), "%d", e->productFrom);
            snprintf(pT, sizeof(pT), "%d", e->productTo);
            fprintf(fo4, "%s\t%s\t%lf\n", pF, pT, e->weight);
        }

        else
        {
            snprintf(pF, sizeof(pF), "%d", e->productFrom);
            snprintf(pT, sizeof(pT), "%d", e->productTo);
            fprintf(fo1, "%s\t%s\t%lf\n", pF, pT, e->weight);

            this->total_train++;
        }
    }
    fclose(fo1);

    printf("Total train: %d\n",this->total_train);

    FILE *fo2 = fopen("validationFile", "wb");
    if (fo2 == NULL)
	{
		printf("ERROR: Failed to create validation file !\n");
		exit(1);
	}
    for (int i = validStart; i < testStart; i ++)
    {
        edge* e = edges[i];

        char pF[100],pT[100];
        snprintf(pF, sizeof(pF), "%d", e->productFrom);
        snprintf(pT, sizeof(pT), "%d", e->productTo);
        fprintf(fo2, "%s\t%s\t%lf\n", pF, pT, e->weight);

        this->total_validation++;
    }
    fclose(fo2);

    printf("Total validation: %d\n",this->total_validation);

    FILE *fo3 = fopen("testFile", "wb");
    if (fo3 == NULL)
	{
		printf("ERROR: Failed to create test file !\n");
		exit(1);
	}
    for (int i = testStart; i < nEdges; i ++)
    {
        edge* e = edges[i];

        char pF[100],pT[100];
        snprintf(pF, sizeof(pF), "%d", e->productFrom);
        snprintf(pT, sizeof(pT), "%d", e->productTo);
        fprintf(fo3, "%s\t%s\t%lf\n", pF, pT, e->weight);

        this->total_test++;
    }
    fclose(fo3);
    fclose(fo4);

    printf("Total test: %d\n",this->total_test);
}

void topicCorpus::reconstruct(char* trainFilePath, int m_depth, int m_k)
{
    max_depth = m_depth;
    max_k = m_k;
    ReadData(trainFilePath);
    printf("ReadData finished.\n");

	FILE *fo = fopen("ReconstructedTrainFile.txt", "wb");
    if (fo == NULL)
	{
		printf("ERROR: Failed to open ReconstructedTrainFile.txt !\n");
		exit(1);
	}
	int sv, cv, cd, len, pst;
	long long num_edges_renet = 0;
	double cw, sum;
	std::queue<int> node, depth;
	std::queue<double> weight;

	std::map<std::string, int>::iterator iter1;
	std::map<std::string, int>::iterator iter2;

	for (std::map<int, ClassVertex_r*>::iterator it = V_r.begin(); it != V_r.end(); it ++)
	{
	    iter1 = corp->itemIds.find(it->second->name);
        int sv = iter1->second;

		if (sv % 10 == 0)
		{
			printf("%cProgress: %.3lf%%", 13, (float)sv / (float)(V_r.size() + 1) * 100);
			fflush(stdout);
		}

		while (!node.empty()) node.pop();
		while (!depth.empty()) depth.pop();
		while (!weight.empty()) weight.pop();
		vid2weight.clear();

		for (std::map<int, ClassVertex_r*>::iterator it = V_r.begin(); it != V_r.end(); it ++) // initialization
		{
		    iter2 = corp->itemIds.find(it->second->name);
            int i = iter2->second;
			rank_list[i].vid = i;
			rank_list[i].weight = 0;
		}

		len = neighbor[sv].size();
		if (len > max_k)
		{
			for (int i = 0; i != len; i++)
				fprintf(fo, "%d\t%d\t%lf\n", sv, neighbor[sv][i].vid, neighbor[sv][i].weight);
			num_edges_renet += len;
			continue;
		}

		vid2weight[sv] += V_r[sv]->degree / 10.0; // Set weights for self-links here!

//		len = neighbor[sv].size();
		sum = V_r[sv]->sum_weight;

		node.push(sv);
		depth.push(0);
		weight.push(sum);

		while (!node.empty())
		{
			cv = node.front();
			cd = depth.front();
			cw = weight.front();

			node.pop();
			depth.pop();
			weight.pop();

			if (cd != 0) vid2weight[cv] += cw;

			if (cd < max_depth)
			{
				len = neighbor[cv].size();
				sum = V_r[cv]->sum_weight;

				for (int i = 0; i != len; i++)
				{
					node.push(neighbor[cv][i].vid);
					depth.push(cd + 1);
					weight.push(cw * neighbor[cv][i].weight / sum);
				}
			}
		}

		pst = 0;
		std::map<int, double>::iterator iter;
		for (iter = vid2weight.begin(); iter != vid2weight.end(); iter++)
		{
			rank_list[pst].vid = (iter->first);
			rank_list[pst].weight = (iter->second);
			pst++;
		}
		std::sort(rank_list, rank_list + pst);

		for (int i = 0; i != max_k; i++)
		{
			if (i == pst) break;
			fprintf(fo, "%d\t%d\t%.6lf\n", sv, rank_list[i].vid, rank_list[i].weight);
			num_edges_renet++;

			int bid1 = corp->itemIds[V_r[sv]->name];
			int bid2 = corp->itemIds[V_r[rank_list[i].vid]->name];
			if (corp->productGraphs[0].find(make_pair(bid1,bid2)) == corp->productGraphs[0].end())
            {
                corp->productGraphsLINE.insert(make_pair(bid1,bid2));
                corp->edgeAdjGraphs[0][bid1].push_back(bid2);
                corp->edgeMap[make_pair(bid1,bid2)] = new int [G];
                corp->edgeMap[make_pair(bid1,bid2)][0] = 1;
            }
		}
	}
	printf("\n");
	fclose(fo);

	printf("Number of edges in reconstructed network: %lld\n", num_edges_renet);
}
