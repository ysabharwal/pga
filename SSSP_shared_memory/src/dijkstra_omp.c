#include <omp.h>
#include "../include/util.h"

int *dijkstra(int n, WTYPE **w, int r, int num_threads) {
    /* Requirement: schedule(static, n / num_threads) */
    if (num_threads > n) num_threads = n;

    int *T = (int *) malloc(n * sizeof(int));
    int *d = (int *) malloc(n * sizeof(int));

    /* Initializing the tree T = {r} */
    for (int v = 0; v < n; ++v)
         T[v] = v == r;

    /* Initializing tentative distances d*/
    d[r] = 0;
    for (int v = 0; v < n; ++v)            /* d[v in V \ T] = w[r][v] */
         if (!T[v])
                d[v] = w[r][v];

    /* Adding other vertices to T */
    for (int i = 0; i < n - 1; ++i) {
         /* Next vertex to add in T */
        int d_u = INF_D;
        #pragma omp parallel for reduction(min : d_u) \
        num_threads(num_threads) default(shared) \
        schedule(static, n / num_threads)
        for (int v = 0; v < n; ++v)        /* find d_u = min{d[v] | v in V \ T} */
            if (!T[v] && d[v] < d_u)
                d_u = d[v];

        int u = INF_U;
        #pragma omp parallel for reduction(min : u) \
        num_threads(num_threads) default(shared) \
        schedule(static, n / num_threads)
        for (int v = 0; v < n; ++v)        /* find u st d[u] = d_u = min{d[v] | v in V \ T} */
            if (!T[v] && d[v] == d_u)
                u = v;

        /* If the graph is disconnected then after
           * a point no vertex can be reached */
        if (u == INF_U) break;

        /* Add u to T */
         T[u] = 1;                         /* T = T union {u} */

        /* Relaxation of neighbors of u */
        #pragma omp parallel for num_threads(num_threads) default(shared) schedule(static, n / num_threads)
        for (int v = 0; v < n; ++v) {
                if (!T[v] && d[v] > d[u] + w[u][v])
                    d[v] = d[u] + w[u][v]; /* d[v in V \ T] = min{d[v], d[u] + w[u, v]} */
         }

    }

    free(w);
    free(T);

    return d;
}

int main(int argc, char **argv) {
    int n, r;
    WTYPE **w = read_input(argv[1], &n, &r);

    double start_time = omp_get_wtime();                           /* Noting time */

    int num_threads = atoi(argv[3]);
    int *d = dijkstra(n, w, r, num_threads);

    printf("OMP time: %f s\n", omp_get_wtime() - start_time);  /* Noting time */

    write_output(argv[2], d, n);

    free(d);
    
    return 0;
}