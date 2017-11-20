#include "topicCorpus.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <map>
#include <queue>
using namespace std;

long long num_vertices = 0;
long long vector_dim2;
int binary;

void topicCorpus::ReadVector(char* input_file2)
{
	char ch, ids[MAX_STRING];
        string str;
	float f_num;
	long long l;
	int id;
	std::map<std::string, int>::iterator rviter;

	FILE *fi = fopen(input_file2, "rb");
	if (fi == NULL) {
		printf("Vector file 2 not found\n");
		exit(1);
	}

	fscanf(fi, "%lld\t%lld", &num_vertices, &vector_dim2);

	printf("num_vertices = %lld vector_dim2 = %d\n",num_vertices,vector_dim2);fflush(stdout);

	vec2 = (float *)calloc(vector_dim2, sizeof(float)); //for tvt file
	vec2[55]=0.98;
	printf("vec2 size = %d\n",nItems * vector_dim2);
	for (long long k = 0; k != num_vertices; k++)
	{
        	fscanf(fi, "%s", ids);
		ch = fgetc(fi);

	        id = atoi(ids);

		l = id * vector_dim2;
		printf("id=%d l=%d\nf_num=",id,l);fflush(stdout);
		for (int c = 0; c != vector_dim2; c++)
		{
		    	fread(&f_num, sizeof(float), 1, fi);
			printf("%lf ",f_num);fflush(stdout);
			vec2[c + l] = f_num;
		}
		printf("\n");
	}
	fclose(fi);

	printf("Vocab size: %lld\n", num_vertices);
	printf("Vector size 2: %lld\n", vector_dim2);
}

void topicCorpus::concatenate(char* input_file2)
{
	ReadVector(input_file2);
}
