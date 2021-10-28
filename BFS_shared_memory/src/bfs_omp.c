#include <util.h>
#include <omp.h>

void
parallel_bfs(int n, vec_int* adjList, int src, int num_threads)
{
  if (num_threads > n)
    num_threads = n;
  int *lvl = (int*) malloc(n * sizeof(int));

  for (int i = 0; i < n; ++i) {
    lvl[i] = -1;
  }

  lvl[src] = 0;
  int curr_lvl = 0;
  int* frontier = (int*)malloc(sizeof(int));
  int frontier_size = 0;
  int* next_frontier = NULL;
  int next_frontier_size = 0;

  frontier[0] = src;
  frontier_size++;

  int* local_frontier_sizes = (int*) malloc(num_threads * sizeof(int));

  while (frontier_size) {

    #pragma omp parallel default(shared) num_threads(num_threads) schedule(dynamic)
    {
      int tid = omp_get_thread_num();
      vec_int local_next_frontier = vec_int_init();

      #pragma omp for
      for (int i = 0; i < frontier_size; ++i) {
        int u = frontier[i];

        foreach(vec_int, &adjList[u], it_) {
          int v = *it_.ref;

          int insert;
          if (lvl[v] == -1) {
            #pragma omp atomic capture
            {
              insert = lvl[v];
              lvl[v] = curr_lvl + 1;
            }

            if (insert == -1)
              vec_int_push_back(&local_next_frontier, v);
          }
        }
      }

      local_frontier_sizes[tid] = local_next_frontier.size;
      #pragma omp atomic update
      {
        next_frontier_size += local_next_frontier.size;
      }
      #pragma omp barrier

      #pragma omp single
      {
        next_frontier = (int*) malloc(next_frontier_size * sizeof(int));
      }


      // Merge the local_next_frontiers
      int start = 0;
      for (int i = 0; i < tid; ++i) {
        start += local_frontier_sizes[i];
      }

      for (int i = start; i < start + local_next_frontier.size; ++i) {
        next_frontier[i] = local_next_frontier.value[i - start];
      }
    }


    frontier = next_frontier;
    frontier_size = next_frontier_size;
    curr_lvl++;
    next_frontier_size = 0;
  }

  for (int i = 0; i < n; ++i) {
    printf("%d\t%d\n", i, lvl[i]);
  }

  free(lvl);
  free(adjList);
  free(local_frontier_sizes);
}


int
main(int argc, char* argv[])
{
  char* filename = argv[1];
  int n = 0, src;
  vec_int* adjList = read_dataset(filename, &n, &src);

  int num_threads = atoi(argv[2]);

  parallel_bfs(n, adjList, src, num_threads);
  return 0;
}
