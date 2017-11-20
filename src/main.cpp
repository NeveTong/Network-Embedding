#include "corpus.hpp"
#include "topicCorpus.hpp"
using namespace std;

int main(int argc, char** argv)
{
  srand(0);
  printf("entered main.\n");
  if (argc < 2)
  {
    printf("An input file is required\n");
    exit(0);
  }

  if (argc < 16)
  {
    printf("Arguments required are\n");
    printf("  1: review corpus to use\n");
    printf("  2: lambda = tradeoff between logistic regressor and topic model\n");
    printf("  3: bandwidth = l2 regularizer on parameters\n");
    printf("  4: productsPerTopic = number of instances of a subcategory we have to observe before adding another topic dimension\n");
    printf("  5: category metadata file\n"); //product-category
    printf("  6: file containing potential duplicates to be merged\n");
    printf("  7: graph\n");

    printf("  8: train file\n");
    printf("  9: depth\n");
    printf("  10: threshold\n");
    printf("  11: binary\n");
    printf("  12: size\n");
    printf("  13: order\n"); // 0:all
    printf("  14: negative\n");
    printf("  15: rho\n");
    printf("  16: threads\n");
  }

  double lambda = atof(argv[2]);
  double bandwidth = atof(argv[3]);
  int productsPerTopic = atof(argv[4]);
  char* categoryPath = argv[5];
  char* duplicatePath = argv[6];

  char* trainFilePath = argv[8];
  int max_depth = atoi(argv[9]);
  int max_k = atoi(argv[10]);


  printf("{\n");
  printf("  \"corpus\": \"%s\",\n", argv[1]);
  printf("  \"lambda\": %f,\n", lambda);

  printf("  \"depth\": %d,\n", max_depth);  //
  printf("  \"threshold\": %d,\n", max_k);  //
  printf("  \"binary\": %d,\n", atoi(argv[11]));
  printf("  \"size\": %d,\n", atoi(argv[12]));  //
  printf("  \"order\": %d,\n", atoi(argv[13]));
  printf("  \"negative\": %d,\n", atoi(argv[14]));  //
  printf("  \"rho\": %lf,\n", atof(argv[15]));  //
  printf("  \"threads\": %d,\n", atoi(argv[16]));

  int G = 1;
  corpus corp(argv + 7, G); // top 5000 frequency words in reviews; user & item statistics; vote structure(common.h);
  topicCorpus ec(&corp,
                 G, // Number of graphs
                 lambda,
                 bandwidth,
                 atoi(argv[11]),
                 atoi(argv[12]),
                 atoi(argv[13]),
                 atoi(argv[14]),
                 0.025,
                 atoi(argv[16])
                 ); 
  //printf("ec finished.\n");
  ec.init();
  //printf("init finished.\n");
  ec.initEdges();
  //printf("initEdges finished.\n");

  printf("}\n");

/*
  ec.createTVTFile(trainFilePath);
  printf("createTVTFile finished.\n");
  ec.reconstruct(trainFilePath,max_depth,max_k);
  printf("reconstruct finished.\n");
  ec.line(ec.total_train);
  printf("line finished.\n");*/
  ec.concatenate("/home/auburn/Documents/data/vec_2nd_trainFile_bought_together.txt");
  printf("concatenate finished.\n");
  ec.validation();
  printf("validation finished.\n");


  return 0;
}

