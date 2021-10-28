#include <mpi.h>
#include "../include/util.h"

#define INDEX(__v__) v_to_index(__v__, my_first_v, my_num_v)
#define OWNER(__v__) (INDEX(__v__) != -1)
#define VERTEX(__i__) index_to_v(__i__, my_first_v, my_num_v)

int index_to_v(int index, int my_first_v, int my_num_v) {
    /* Returns the vertex v at index 'index' in the local w matrix
     * of a processor that owns my_num_v vertices starting from my_first_v */
    return index + my_first_v;
}

int v_to_index(int v, int my_first_v, int my_num_v) {
    /* Returns the index of vertex v in the local w matrix
     * of a processor that owns my_num_v vertices starting from my_first_v */
    int candidate = v - my_first_v;
    if (candidate >= 0 && candidate < my_num_v)
        return candidate;
    return -1;
}

int *dijkstra(int n, WTYPE **my_w, int r, int my_rank, int num_proc) {
    /* Getting vertices owned by this processor */
    int my_first_v = (n / num_proc) * my_rank + ((my_rank < n % num_proc) ? my_rank : n % num_proc);
    int my_num_v = n / num_proc + (my_rank < n % num_proc);

    int *my_T = (int *) malloc(my_num_v * sizeof(int));
    int *my_d = (int *) malloc(my_num_v * sizeof(int));

    /* Initializing the tree T = {r} */
    for (int v = 0; v < my_num_v; ++v)
        my_T[v] = VERTEX(v) == r;

    /* Initializing tentative distances d */
    if (OWNER(r)) my_d[INDEX(r)] = 0;
    for (int v = 0; v < my_num_v; ++v)     /* d[v in V \ T] = w[r][v] */
        if (!my_T[v]) my_d[v] = my_w[r][v];

    /* Adding other vertices to T */
    for (int i = 0; i < n - 1; ++i) {

        /* Next vertex to add in T */
        int my_u = -1;
        int my_d_u = INF_D;

        for (int v = 0; v < my_num_v; ++v) {
            if (!my_T[v] && my_d[v] < my_d_u) {
                my_d_u = my_d[v];
                my_u = VERTEX(v);
            }
        }

        int my_d_u__u[2] = {my_d_u, my_u};
        int d_u__u[2];

        MPI_Allreduce(my_d_u__u, d_u__u, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);

        int d_u = d_u__u[0];
        int u = d_u__u[1];

        /* If the graph is disconnected then after
           * a point no vertex can be reached */
        if (u == -1) break;

        /* Add u to T (done by owner of u) */
        if (OWNER(u)) my_T[INDEX(u)] = 1;                      /* T = T union {u} */

        /* Relaxation of neighbors of u (done by all processors) */
        for (int v = 0; v < my_num_v; ++v)                     /* d[v in V \ T] = w[r][v] */
            if (!my_T[v] && my_d[v] > d_u + my_w[u][v])
                my_d[v] = d_u + my_w[u][v];                    /* d[v in V \ T] = min{d[v], d[u] + w[u, v]} */

    }

    free(my_w);
    free(my_T);

    return my_d;
}

WTYPE **read_my_input(char input_file_name[], int *n_, int *r_, int my_rank, int num_proc) {
    /* Each processor reads the input file and constructs it's my_w */
    FILE *input_file = fopen(input_file_name, "r");
    
    // Reading #vertices, #edges, root, nature of graph (directed/undirected)
    int m = 0, undir = 0;
    fscanf(input_file, "%d %d %d %d \n", n_, &m, r_, &undir);
    int n = *n_;
    int my_first_v = (n / num_proc) * my_rank + ((my_rank < n % num_proc) ? my_rank : n % num_proc);
    int my_num_v = n / num_proc + (my_rank < (n % num_proc));
    
    // Initializing *my_w (adjacency matrix columns)
    WTYPE **my_w = (int **) malloc(n * sizeof(int *));
    for (int i = 0; i < n; ++i) {
        my_w[i] = (int *) malloc(n * sizeof(int));
        for (int j = 0; j < my_num_v; ++j)
            my_w[i][j] = INF_W;
    }
    // Filling the adjacency matrix
    for (int i = 0; i < m; ++i) {
        int u, v, uv_w;
        fscanf(input_file, "%d %d %d \n", &u, &v, &uv_w);
        if (OWNER(v))
            my_w[u][INDEX(v)] = uv_w;
        if (OWNER(u))
            my_w[v][INDEX(u)] = (undir) ? uv_w : my_w[v][INDEX(u)];
    }

    fclose(input_file);

    return my_w;
}

void gather_and_write_output(char output_file_name[], int *my_d, int n, int my_rank, int num_proc) {
    /* Parameters for gatherv */
    int disp[num_proc];              /* Denotes the displacement */
    int d_sz[num_proc];                /* Denotes the size of input for each processor */
    int d_[n];
    int my_first_v = (n / num_proc) * my_rank + ((my_rank < n % num_proc) ? my_rank : n % num_proc);
    int my_num_v = n / num_proc + (my_rank < (n % num_proc));

    if (my_rank == 0) {                                        /* Master prepares fills the (in) parameters */
        /* Filling d_sz and disp */
        for (int proc_num = 0; proc_num < num_proc; ++proc_num) {
            d_sz[proc_num] = n / num_proc + (proc_num < (n % num_proc));
            disp[proc_num] = (proc_num == 0) ? 0 : (disp[proc_num - 1] + d_sz[proc_num - 1]);
        }
    }


    MPI_Gatherv(my_d, my_num_v, MPI_INT, d_, d_sz, disp, MPI_INT, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        write_output(output_file_name, &d_, n);
    }
}



int main(int argc, char **argv) {
    double start_time, end_time;
    MPI_Init(&argc, &argv);                    /* initialize MPI operations */

    int my_rank;                               /* process my_rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);   /* get my_rank */
    int num_proc = atoi(argv[3]);

    int n, r;
    WTYPE **my_w = read_my_input(argv[1], &n, &r, my_rank, num_proc);

    if (my_rank == 0) start_time = MPI_Wtime();/* Noting time */

    int *my_d = dijkstra(n, my_w, r, my_rank, num_proc);

    if (my_rank == 0)                          /* Noting time */
        printf("MPI time: %f s\n", MPI_Wtime() - start_time);
    
    gather_and_write_output(argv[2], my_d, n, my_rank, num_proc);

    free(my_d);

    MPI_Finalize(); //finalize MPI operations
    return 0;
}
