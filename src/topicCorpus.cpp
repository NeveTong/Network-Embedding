#include "topicCorpus.hpp"
using namespace std;

void topicCorpus::initEdges()
{
  edges = vector<edge*>();

  V = map<int, ClassVertex*>();
  V_r = map<int, ClassVertex_r*>();

  fvecs_edge = new map<int, double> [nEdges];
  fvecs_direction = new map<int, double> [nEdges];
  fvecs_direction_reverse = new map<int, double> [nEdges];

  int i = 0;

  for (map<pair<int,int>, int*>::iterator it = corp->edgeMap.begin(); it != corp->edgeMap.end(); it ++)
  {
    int* labels = it->second;
    int productFrom = it->first.first;
    int productTo = it->first.second;

    map<int, double>* fvec_edge = fvecs_edge + i;
    map<int, double>* fvec_direction = fvecs_direction + i;
    map<int, double>* fvec_direction_reverse = fvecs_direction_reverse + i;

    int weight;
    if(labels == 0) weight = 0;
    else    weight = 1;

    i ++;

    int* reverseLabels = emptyLabel;
    if (corp->edgeMap.find(make_pair(productTo, productFrom)) != corp->edgeMap.end())
      reverseLabels = corp->edgeMap[make_pair(productTo, productFrom)];
    edge* e = new edge(it->first.first,
                       it->first.second,
                       labels,
                       reverseLabels,
                       fvec_edge,
                       fvec_direction,
                       fvec_direction_reverse,
                       weight,
                       G,
                       nLogisticWeights_edge,
                       nLogisticWeights_direction);

    edges.push_back(e);
  }

  random_shuffle(edges.begin(), edges.end());
}
