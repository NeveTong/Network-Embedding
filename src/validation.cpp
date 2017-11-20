#include "topicCorpus.hpp"

int dim_v;

float innerLINE(float *vec_u, float *vec_v)
{
    float res = 0;
    for (int c = 0; c != dim_v; c++)  res += vec_u[c] * vec_v[c];
    return res;
}

void topicCorpus::PrecisionRecall(char* PRPath, int limit) // limit: how many times we try; test with 'A' graphname B C D
{
  fprintf(stderr, "LINE: Saving PR curve to %s\n", PRPath);
  if (PRPath)
  {
    unsigned int* seedptrs = new unsigned int [omp_get_max_threads()];
    for (int s = 0; s < omp_get_max_threads(); s ++)
      seedptrs[s] = s;

    FILE* f = fopen_(PRPath, "w");
    FILE* f1 = fopen_("Precision_Results", "w");

    std::set<std::pair<int,int> > edgesInTrain;
    for (int i = 0; i < validStart; i ++)
    {
      edge* e = edges[i];
      edgesInTrain.insert(std::make_pair(e->productFrom, e->productTo)); // before reconstruct
    }

    fprintf(f, "{\n");
    for (int g = 0; g < G; g ++)
    {
      std::vector<int> validKeys; // 'A' graphname B C D
      for (std::map<int, std::vector<int> >::iterator it = corp->edgeAdjGraphs[g].begin(); it != corp->edgeAdjGraphs[g].end(); it ++)
        validKeys.push_back(it->first);

      int limitG = limit;
      if ((int) validKeys.size() <= limit)
        limitG = validKeys.size();

      std::vector<std::pair<float,int> >* scores = new std::vector<std::pair<float, int> > [limitG];
      std::vector<char>* mapChars = new std::vector<char> [limitG];
      int* choices = new int [limitG];

      fprintf(f, "  \"%s\":\n", corp->graphNames[g].c_str());
      fprintf(f, "    {\n");

#pragma omp parallel for
      for (int i = 0; i < limitG; i ++)
      {
        int sum = 0, hit = 0;
        float precision = 0;
        unsigned int seedptr = seedptrs[omp_get_thread_num()];
        int ip = validKeys[i % validKeys.size()];
        choices[i] = ip;
        if ((int) validKeys.size() > limitG) // If we're taking more samples than there are test examples, just compute scores for all test examples
          ip = validKeys[(rand_r(&seedptr) % validKeys.size())];
        if (i > 0 and i % 99 == 0)
        {
          fprintf(stderr, "p2");
          fflush(stderr);
        }

        for (int jp = 0; jp < nItems; jp ++)
        {
          if (ip == jp) continue;
          if (edgesInTrain.find(std::make_pair(ip,jp)) != edgesInTrain.end() || corp->productGraphsLINE.find(std::make_pair(ip,jp)) != corp->productGraphsLINE.end()) 
		continue; // Skip edges that appeared at training time

          int ipvec2 = id_vec2[ip], jpvec2 = id_vec2[jp];
          float p = innerLINE(&vec2[ipvec2 * dim_v], &vec2[jpvec2 * dim_v]);

          scores[i].push_back(std::make_pair(p,jp));
        }
        sort(scores[i].begin(), scores[i].end());
        for (std::vector<std::pair<float,int> >::iterator it = scores[i].begin(); it != scores[i].end(); it ++)
        {
          int jp = it->second;
          sum ++;
          if (corp->edgeMap.find(std::make_pair(ip,jp)) != corp->edgeMap.end() and corp->edgeMap[std::make_pair(ip,jp)][g])
          {
              mapChars[i].push_back('1');
              hit ++;
          }

          else
            mapChars[i].push_back('0');
        }
        precision = hit/sum;
        fprintf(f1, "%d %d %lf\n", sum, hit, precision);
      }

      for (int i = 0; i < limitG; i ++)
      {
        fprintf(f, "      \"%s\": \"", corp->rItemIds[choices[i]].c_str());
        for (std::vector<char>::iterator it = mapChars[i].begin(); it != mapChars[i].end(); it ++)
          fprintf(f, "%c", *it);
        fprintf(f, "\"");
        if (i < limitG - 1) fprintf(f, ",");
        fprintf(f, "\n");
      }
      fprintf(f, "    }");
      if (g < G - 1)
        fprintf(f, ",");
      fprintf(f, "\n");
      delete [] scores;
      delete [] mapChars;
      delete [] choices;
    }
    fprintf(f, "}\n");
    fprintf(stderr, "\n");

    fclose(f);
    delete [] seedptrs;
  }
}

void topicCorpus::validation()
{
    dim_v = dim;

    PrecisionRecall("LINEPrecisionRecall",9);
}
