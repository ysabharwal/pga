#include <stdio.h>
#include <stdlib.h>

#define P
#define T int
#include <vec.h>

vec_int*
read_dataset(char* dataset, int* n, int *src)
{
  FILE* input;
  char* line;

  input = fopen(dataset, "r");
  if(input == NULL){
    printf("Error in opening %s\n", dataset);
    return NULL;
  }

  // Reading #vertices, #edges, start vertex, nature of graph (directed/undirected)
  int m = 0, undir = 0;
  fscanf(input, "%d %d %d %d\n", n, &m, src, &undir);

  // Initializing the Adjancency List -- Array of vectors
  vec_int* adjList = (vec_int*) malloc(*n * sizeof(vec_int));
  for (int i = 0; i < *n; ++i) {
    adjList[i] = vec_int_init();
  }

  // Filling the adjacency list
  for (int i = 0; i < m; ++i) {
    int u, v, uv_w;
    fscanf(input, "%d %d %d\n", &u, &v, &uv_w);
    vec_int_push_back(&adjList[u], v);
    if (undir)
      vec_int_push_back(&adjList[v], u);
  }

  fclose(input);

  return adjList;
}
