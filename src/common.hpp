#pragma once

#include "stdio.h"
#include "stdlib.h"
#include "vector"
#include "math.h"
#include "string.h"
#include <string>
#include <iostream>
#include "omp.h"
#include "map"
#include "set"
#include "vector"
#include "common.hpp"
#include "algorithm"
#include "sstream"
#include "gzstream.h"

/// Safely open a file
FILE* fopen_(const char* p, const char* m);

/// Inner product of a sparse vector and a dense vector
double inner(std::map<int, double>* x, double* w);

class edge
{
public:
  edge(int productFrom,
       int productTo,
       int* labels, // Labels for each graph type
       int* reverseLabels, // What are the labels of the edge going in the other direction (productTo to productFrom)
       std::map<int, double>* feature_edge,
       std::map<int, double>* feature_direction,
       std::map<int, double>* feature_direction_reverse, // Features of the edge going from productTo to productFrom
       double weight, // cosine similarity between theta
       int G, // Number of graphs
       int F_edge, // Dimensionality of edge features
       int F_direction); // Dimensionality of direction features

  ~edge();

  int productFrom;
  int productTo;

  int* labels;
  int* reverseLabels;
  std::map<int, double>* feature_edge;
  std::map<int, double>* feature_direction;
  std::map<int, double>* feature_direction_reverse;
  double weight; // cosine similarity between theta

  int G;
  int F_edge;
  int F_direction;
};
