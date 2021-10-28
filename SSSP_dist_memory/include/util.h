#include <stdio.h>
#include <stdlib.h>

#define INF_W 10000
#define INF_D 1000000000
#define INF_U 1000000000

typedef short WTYPE;

WTYPE **read_input(char input_file_name[], int *n_, int *r_) {
    FILE *input_file = fopen(input_file_name, "r");
    
    // Reading #vertices, #edges, root, nature of graph (directed/undirected)
    int m = 0, undir = 0;
    fscanf(input_file, "%d %d %d %d \n", n_, &m, r_, &undir);
    int n = *n_;
    
    // Initializing *w (adjacency matrix)
    WTYPE **w = (int **) malloc(n * sizeof(int *));
    for (int i = 0; i < n; ++i) {
        w[i] = (int *) malloc(n * sizeof(int));
        for (int j = 0; j < n; ++j)
            w[i][j] = INF_W;
    }
    // Filling the adjacency matrix
    for (int i = 0; i < m; ++i) {
        int u, v, uv_w;
        fscanf(input_file, "%d %d %d \n", &u, &v, &uv_w);
        w[u][v] = uv_w;
        w[v][u] = (undir) ? uv_w : w[v][u];
    }

    fclose(input_file);

    return w;
}

void write_output(char output_file_name[], int *d, int n) {
    FILE *f = fopen(output_file_name, "w");

    fprintf(f, "******** v : dist(r, v) ********\n");
    for(int i = 0; i < n; ++i){
        if (d[i] == INF_W) fprintf(f, "%d: %s\n", i, "∞");
        else fprintf(f, "%d: %d\n", i, d[i]);
    }

    fclose(f);
}