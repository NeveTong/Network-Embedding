#include "common.hpp"
using namespace std;

/// Safely open a file
FILE* fopen_(const char* p, const char* m)
{
  FILE* f = fopen(p, m);
  if (!f)
  {
    printf("Failed to open %s\n", p);
    exit(1);
  }
  return f;
}

double inner(map<int, double>* x, double* w)
{
  double res = 0;
  for (map<int, double>::iterator it = x->begin(); it != x->end(); it ++)
    res += it->second * w[it->first];
  return res;
}

edge::edge(int productFrom,
           int productTo,
           int* labels,
           int* reverseLabels,
           map<int, double>* feature_edge,
           map<int, double>* feature_direction,
           map<int, double>* feature_direction_reverse,
           double weight,
           int G,
           int F_edge,
           int F_direction) :
    productFrom(productFrom),
    productTo(productTo),
    labels(labels),
    reverseLabels(reverseLabels),
    feature_edge(feature_edge),
    feature_direction(feature_direction),
    feature_direction_reverse(feature_direction_reverse),
    weight(weight),
    G(G),
    F_edge(F_edge),
    F_direction(F_direction)
  {}
