#include <util.h>
#include <mpi.h>

#define INDEX(__v__) __v__ / size
#define OWNER(__v__) __v__ % size
#define VERTEX(__i__) __i__ * size + my_rank

int
max(int x, int y)
{
  return x > y ? x : y;
}

vec_int*
read_dataset_local(char* dataset, int* n, int *src, int size, int my_rank)
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
  int my_num_v = (*n) / size + (my_rank < *n % size);

  vec_int* adjList = (vec_int*) malloc(my_num_v * sizeof(vec_int));
  for (int i = 0; i < my_num_v; ++i) {
    adjList[i] = vec_int_init();
  }

  // Filling the local adjacency list
  for (int i = 0; i < m; ++i) {
    int u, v, uv_w;
    fscanf(input, "%d %d %d\n", &u, &v, &uv_w);
    if (OWNER(u) == my_rank)
      vec_int_push_back(&adjList[INDEX(u)], v);
    if (undir && OWNER(v) == my_rank)
      vec_int_push_back(&adjList[INDEX(v)], u);
  }

  fclose(input);

  return adjList;

}


void
distributed_bfs_1D(int n, vec_int* adjList, int src, int size, int my_rank)
{
  vec_int frontier = vec_int_init();
  vec_int next_frontier = vec_int_init();

  int curr_lvl = 0;
  int my_num_v = n / size + (my_rank < n % size);

  int *lvl = (int*) malloc(my_num_v * sizeof(int));

  for (int i = 0; i < my_num_v; ++i) {
    lvl[i] = -1;
  }

  if (OWNER(src) == my_rank) {
    lvl[INDEX(src)] = 0;
    vec_int_push_back(&frontier, src);
  }

  vec_int* Buffer = (vec_int*) malloc(size * sizeof(vec_int));

  for (int i = 0; i < size; ++i) {
    Buffer[i] = vec_int_init();
  }

  while (1) {

    for (int i = 0; i < size; ++i)
      vec_int_clear(&Buffer[i]);

    for (int i = 0; i < frontier.size; ++i) {
      int u = frontier.value[i];

      for (int j = 0; j < adjList[INDEX(u)].size; ++j) {
        int v = adjList[INDEX(u)].value[j];

        int w = OWNER(v);
        vec_int_push_back(&Buffer[w], v);

      }
    }

    // AlltoAllv
    int send_counts[size];
    for (int i = 0; i < size; ++i) {
      send_counts[i] = max(1, Buffer[i].size);
    }

    int recv_counts[size];
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int send_disp[size];
    int recv_disp[size];

    int send_buffer_size = 0, recv_buffer_size = 0;

    for (int i = 0; i < size; ++i) {
      send_disp[i] = send_buffer_size;
      recv_disp[i] = recv_buffer_size;
      send_buffer_size += send_counts[i];
      recv_buffer_size += recv_counts[i];
    }

    int* send_buf = (int*) malloc(send_buffer_size * sizeof(int));
    int* recv_buf = (int*) malloc(recv_buffer_size * sizeof(int));

    // Fill the send buffer
    for (int i = 0; i < size; ++i) {
      int index = send_disp[i];
      if (Buffer[i].size == 0){
        send_buf[index] = -1;
      }
      else {
        foreach(vec_int, &Buffer[i], it) {
          send_buf[index] = *it.ref;
          index++;
        }
      }

    }

    MPI_Alltoallv(send_buf, send_counts, send_disp, MPI_INT,
                  recv_buf, recv_counts, recv_disp, MPI_INT,
                  MPI_COMM_WORLD);

    for (int p = 0; p < size; ++p) {
      for (int i = recv_disp[p]; i < recv_disp[p] + recv_counts[p]; ++i) {
        if (recv_buf[i] != -1)
          vec_int_push_back(&next_frontier, recv_buf[i]);
      }
    }

    vec_int_clear(&frontier);

    foreach(vec_int, &next_frontier, it) {
      int v = *it.ref;

      if (lvl[INDEX(v)] == -1) {
        lvl[INDEX(v)] = curr_lvl + 1;
        vec_int_push_back(&frontier, v);
      }
    }

    vec_int_clear(&next_frontier);
    curr_lvl++;

    int size = frontier.size;
    int total_size;
    MPI_Allreduce(&size, &total_size, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    free(send_buf);
    free(recv_buf);

    if (total_size == 0)
      break;

  }

  for (int p = 0; p < size; ++p) {
    if (my_rank == p){
      for (int i = 0; i < my_num_v; ++i) {
        printf("%d\t%d\n", VERTEX(i), lvl[i]);
      }
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }


  free(lvl);
  free(adjList);
  free(Buffer);

}


int
main(int argc, char* argv[])
{
  MPI_Init(&argc, &argv);

  char* filename = argv[1];


  int size, my_rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int n = 0, src;
  vec_int* adjList = read_dataset_local(filename, &n, &src, size, my_rank);

  distributed_bfs_1D(n, adjList, src, size, my_rank);

  MPI_Finalize();

  return 0;
}
