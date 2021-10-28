#include "../include/util.h"
#include <time.h>

int *dijkstra(int n, WTYPE **w, int r) {
    int *T = (int *) malloc(n * sizeof(int));
    int *d = (int *) malloc(n * sizeof(int));

    /* Initializing the tree T = {r} */
    for (int v = 0; v < n; ++v)
        T[v] = v == r;

    /* Initializing tentative distances d*/
    d[r] = 0;
    for (int v = 0; v < n; ++v)           /* d[v in V \ T] = w[r][v] */
        if (!T[v])
            d[v] = w[r][v];

    /* Adding other vertices to T */
    for (int i = 0; i < n - 1; ++i) {
        /* Next vertex to add in T */
        int u = -1, d_u = INF_D;
        for (int v = 0; v < n; ++v) {    /* find u st d[u] = min{d[v] | v in V \ T} */
            if (!T[v] && d[v] < d_u) {
                d_u = d[v];
                u = v;
            }
        }

        /* If the graph is disconnected then after
         * a point no vertex can be reached */
        if (u == -1) break;

        /* Add u to T */
        T[u] = 1;                        /* T = T union {u} */

        /* Relaxation of neighbors of u */
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

    clock_t start_time = clock();                                 /* Noting time */

    int *d = dijkstra(n, w, r);

    printf("Sequential time: %f s\n", ((double)(clock() - start_time)) / CLOCKS_PER_SEC);  /* Noting time */

    write_output(argv[2], d, n);

    free(d);

    return 0;
}