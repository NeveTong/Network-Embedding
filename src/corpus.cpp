#include "corpus.hpp"
#include "common.hpp"
#include <boost/algorithm/string/trim.hpp>
using namespace std;

corpus::corpus(char** graphPaths, int G) :
    G(G)
{
  printf("enter corp.\n");
  loadGraphs(graphPaths);
}

/// Parse G product graphs
void corpus::loadGraphs(char** graphPaths)
{
  printf("enter loadGraphs.\n");
  nEdges = 0;
  productGraphs = new set<pair<int,int> > [G];
  edgeAdjGraphs = new map<int, vector<int> > [G];

  printf("  \"nEdges\": {");

  for (int g = 0; g < G; g ++)
  {
    igzstream in;
    in.open(graphPaths[g]);

    string n1; // source vertex name
    string n2; // target vertex name
    string edgename;

    string line;

    nEdges = 0;
    nItems = 0;
    int show = 0;

    while (getline(in, line))
    {
      stringstream ss(line);
      ss >> n1 >> edgename; // Second word of each line should be the edge type
      show ++;
      if(show%100000==0)
      {
          printf("c"); fflush(stdout);
      }
      //printf("%s\n", n1.c_str());
      if (itemIds.find(n1) == itemIds.end())
      {
        rItemIds[nItems] = n1;
        itemIds[n1] = nItems++;
      }
      int bid1 = itemIds[n1];
      if (edgeAdjGraphs[g].find(bid1) == edgeAdjGraphs[g].end())
        edgeAdjGraphs[g][bid1] = vector<int>();

      while (ss >> n2)
      {
        nEdges ++;
        if (itemIds.find(n2) == itemIds.end())
        {
            rItemIds[nItems] = n2;
            itemIds[n2] = nItems++;
        }
        int bid2 = itemIds[n2];

        productGraphs[g].insert(make_pair(bid1,bid2));
        edgeAdjGraphs[g][bid1].push_back(bid2);

        nodesInSomeEdge.insert(bid1);
        nodesInSomeEdge.insert(bid2);
      }
    }

    graphNames.push_back(edgename);
    printf("\"%s\": %d", edgename.c_str(), nEdges);
    if (g < G - 1) printf(", ");
  }
  printf("},\n");

  for (set<int>::iterator it = nodesInSomeEdge.begin(); it != nodesInSomeEdge.end(); it ++)
    nodesInSomeEdgeV.push_back(*it);
  emptyLabel = new int [G];
  for (int g = 0; g < G; g ++)
    emptyLabel[g] = 0;

  for (int g = 0; g < G; g ++)
  {
    for (set<pair<int,int> >::iterator it = productGraphs[g].begin(); it != productGraphs[g].end(); it ++)
    {
      pair<int,int> e = *it;
      if (edgeMap.find(e) == edgeMap.end())
      {
        edgeMap[e] = new int [G];
        for (int i = 0; i < G; i ++)
          edgeMap[e][i] = 0;
      }
      edgeMap[e][g] = 1;

      pair<int,int> random1(it->first, nodesInSomeEdgeV[rand() % nodesInSomeEdgeV.size()]);
      pair<int,int> random2(nodesInSomeEdgeV[rand() % nodesInSomeEdgeV.size()], it->second);
      pair<int,int> random3(nodesInSomeEdgeV[rand() % nodesInSomeEdgeV.size()], nodesInSomeEdgeV[rand() % nodesInSomeEdgeV.size()]);
    }
  }

  // Generate random non-edges; number of non-edges<=edges
  int nRandom = edgeMap.size();
  for (int r = 0; r < nRandom; r ++)
  {
    int nn = nodesInSomeEdgeV.size();
    int e1 = nodesInSomeEdgeV[rand() % nn];
    int e2 = nodesInSomeEdgeV[rand() % nn];
    pair<int,int> e(e1,e2);

    if (edgeMap.find(e) != edgeMap.end()) continue;
    edgeMap[e] = emptyLabel; // Label is zero for all edge types
  }

  nEdges = edgeMap.size();
}

corpus::~corpus()
{
  delete [] productGraphs;
  delete [] edgeAdjGraphs;
  for (map<pair<int,int>, int*>::iterator it = edgeMap.begin(); it != edgeMap.end(); it ++)
  {
    if (it->second != emptyLabel)
      delete [] it->second;
  }
  delete [] emptyLabel;

}
