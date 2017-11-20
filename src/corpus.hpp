#pragma once


#include "common.hpp"

class corpus
{
public:
  corpus(char** graphPaths, int G);
  ~corpus();

  void loadGraphs(char** graphPaths);

  int nItems; // Number of items
  int nEdges; // Number of edges ; size of edgeMap (come from file & 'random non-edges' in corpus.cpp)
  int G; // Number of graphs

  std::map<std::string, int> itemIds; // Maps an item's string-valued ID to an integer

  std::map<int, std::string> rItemIds;

  std::vector<std::string> graphNames;
  std::set<std::pair<int,int> >* productGraphs; // Edgelist
  std::map<int, std::vector<int> >* edgeAdjGraphs; // Edge-adjacency list
  std::map<std::pair<int,int>, int*> edgeMap; // Maps a pair of products to its labels for each graph (list of length G) ;come from file & 'random non-edges' in corpus.cpp
  int* emptyLabel;
  std::set<std::pair<int,int> > productGraphsLINE; // Edges added in LINE

  std::set<int> nodesInSomeEdge; // Set of nodes that appear in some edge; all items
  std::vector<int> nodesInSomeEdgeV; // Same thing as a vector

};

