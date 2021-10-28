#include <util.h>


void
sequential_bfs(int n, vec_int* adjList, int src)
{
  int *lvl = (int*) malloc(n * sizeof(int));

  for (int i = 0; i < n; ++i) {
    lvl[i] = -1;
  }

  lvl[src] = 0;
  int curr_lvl = 0;
  vec_int frontier = vec_int_init();
  vec_int next_frontier = vec_int_init();

  vec_int_push_back(&frontier, src);

  while (frontier.size) {
    foreach(vec_int, &frontier, it) {
      int u = *it.ref;

      foreach(vec_int, &adjList[u], it_) {
        int v = *it_.ref;

        if (lvl[v] == -1) {
          vec_int_push_back(&next_frontier, v);
          lvl[v] = curr_lvl + 1;
        }

      }
    }

    frontier = next_frontier;
    curr_lvl++;
    next_frontier = vec_int_init();
  }

  for (int i = 0; i < n; ++i) {
    printf("%d\t%d\n", i, lvl[i]);
  }

  free(lvl);
  free(adjList);

}


int
main(int argc, char* argv[])
{
  char* filename = argv[1];
  int n = 0, src;
  vec_int* adjList = read_dataset(filename, &n, &src);

  sequential_bfs(n, adjList, src);
  return 0;
}
