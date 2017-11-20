#pragma once
#include "common.hpp"
#include "corpus.hpp"

#define max_num_vertices 1000
#define hash_table_size 30000000
#define MAX_STRING 100
#define SIGMOID_BOUND 6
#define NEG_SAMPLING_POWER 0.75


struct ClassVertex
{
	double degree;
	std::string name;
};
typedef struct ClassVertex ClassVertex;

struct ClassVertex_r
{
	double degree, sum_weight;
	std::string name;
};
typedef struct ClassVertex_r ClassVertex_r;

class topicCorpus
{
public:
  topicCorpus(corpus* corp, // The corpus
              int G, // The number of graphs
              double lambda, // Topic model regularizer
              double bandwidth,
              int is_binary,
              int dim,
              int order,
              int num_negative,
              float init_rho,
              int num_threads
             ) :
    corp(corp),
    G(G),
    lambda(lambda),
    bandwidth(bandwidth),
    is_binary(is_binary),
    dim(dim),
    order(order),
    num_negative(num_negative),
    init_rho(init_rho)
  {
  }

  void init()
  {
    srand(0);

    nItems = corp->nItems;
    nEdges = corp->nEdges;

    // Use 1/3 of the data for train, 1/3 for validation, up to a maximum of 1M training/validation instances
    double testFraction = 0.1;
    validStart = (int) ((1.0 - 2*testFraction)*nEdges);
    testStart = (int) ((1.0 - testFraction)*nEdges);

    if (validStart < 1 or (testStart - validStart) < 1 or (nEdges - testStart) < 1)
    {
      printf("Didn't get enough edges (%d/%d/%d)\n", validStart, testStart, nEdges);
      exit(1);
    }

    emptyLabel = new int [G];
    for (int g = 0; g < G; g ++)
      emptyLabel[g] = 0;

  }

  ~topicCorpus()
  {

    delete [] emptyLabel;

  }

  void initEdges(void);

  void createTVTFile(char* trainFilePath);
  void reconstruct(char* trainFilePath, int max_depth, int max_k);
  void ReadData(char* trainFilePath);
  void AddVertex_r(int name);

  void line(int total_train);
  void AddVertex_l(int name);
  void ReadData_l();
  void InitVector();
  void InitNegTable();
  void Output_l();
  void TrainLINE();

  void concatenate(char* input_file2);
  void ReadVector(char* input_file2);
  int AddVertex_c(std::string name, int vid);

  void validation(void);
  void PrecisionRecall(char* PRPath, int limit);

  corpus* corp;

  int validStart;
  int testStart;
  int total_train;
  int total_validation;
  int total_test;

/* Build a hash table, mapping each vertex name to a unique vertex id
  std::map<int, int> vertex_hash_table; //vertex_hash_table[Hash(name)]=vids ---> <Hash(name),vids>
  std::map<std::string, int> rvertex_hash_table; // <name,hash> vertex_hash_table[addr] = value
  unsigned int Hash(std::string key);
  void InsertHashTable(std::string key, int value);
  int SearchHashTable(std::string key);
  int SearchHashTable_r(std::string key);
*/

  float *vec2;
  std::map<int, long long> id_vec2;
  std::map<int, ClassVertex*> V;
  std::map<int, ClassVertex_r*> V_r; // V_r.insert(make_pair(num_vertices_r,v_r));
  ClassVertex_r* v_r;
  ClassVertex* v;

  int nLogisticWeights_edge;
  int nLogisticWeights_direction;

  int G; // Number of graphs

  double lambda;
  double bandwidth;
  int is_binary;
  int dim;
  int order;
  int num_negative;
  float init_rho;
  int num_threads;

  int nItems; // Number of items
  int nEdges; // Number of edges

  int* emptyLabel;

  std::map<int, double>* fvecs_edge;
  std::map<int, double>* fvecs_direction;
  std::map<int, double>* fvecs_direction_reverse;;
  std::vector<edge*> edges;

};

extern topicCorpus* gtc;

