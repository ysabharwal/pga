#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define P
#define T int
#include <vec.h>

#define P
#define T int
#include <set.h>

#define P
#define T int
#include <lst.h>

#include <time.h>
#include <sys/time.h>

double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

int compare(int* a, int* b) { return *a == *b ? 0 : *a < *b ? -1 : 1; }

int main(int argc, char *argv[]) {
    if(argc!=4) {
        printf("3 inputs needed\n");
        printf("> input file name\n");
        printf("> reverse mapping file name\n");
        printf("> output file name\n");
        return -1;
    }
    char *filename = argv[1];
    char *reverse_mapping_filename = argv[2];
    char *output_filename = argv[3];
 
    FILE *f = fopen(filename,"r");
    int n,m,source,undirected;
    fscanf(f, "%d\t%d\t%d\t%d\n", &n, &m, &source, &undirected);

    FILE *f2 = fopen(reverse_mapping_filename, "r");
    int reverse_map[n];
    {
        int tempn,tempu,tempv;
        fscanf(f2, "%d\n", &tempn);
        for(int i=0; i<n; i++) {
            fscanf(f2, "%d\t%d\n", &tempu, &tempv);
            reverse_map[tempu] = tempv;
        }
    }
    fclose(f2);

    lst_int *neighbours = malloc(n*sizeof(lst_int));
    for(int i=0; i<n; i++)
        neighbours[i] = lst_int_init();
    int *levels = (int*)malloc(n*sizeof(int));
    for(int i=0; i<n; i++)
        levels[i] = -1;
    levels[source] = 0;

    for(int i=0; i<m; i++) {
        int u,v,uv;
        fscanf(f,"%d\t%d\t%d\n",&u,&v,&uv);
        if(undirected) {
            lst_int_push_back(&neighbours[u],v);
            lst_int_push_back(&neighbours[v],u);
        }
        else {
            lst_int_push_back(&neighbours[u],v);
        }
    }
    fclose(f);

    set_int frontier = set_int_init(compare);
    set_int_insert(&frontier,source);
    set_int next_frontier = set_int_init(compare);
    int curr_level = 0;
    int total_size = 1;

    double bfs_start = get_wall_time();

    while(1) {
        foreach(set_int,&frontier,it) {
            int u = *it.ref;
            //levels[u] = curr_level;
            foreach(lst_int,&neighbours[u],it2) {
                int v = *it2.ref;
                if(levels[v]==-1) {
                    set_int_insert(&next_frontier, v);
                    levels[v] = curr_level+1;
                }
            }
        }

        int size = next_frontier.size;
        total_size += size;
        set_int_clear(&frontier);
        foreach(set_int, &next_frontier, it) {
            set_int_insert(&frontier,*it.ref);
        }
        set_int_clear(&next_frontier);
        curr_level++;
        if(size==0)
            break;

    }

    double bfs_end = get_wall_time();

    f = fopen(output_filename, "w");
    for(int i=0; i<n; i++) {
        fprintf(f,"%d\t%d\n",reverse_map[i],levels[i]);
    }
    fclose(f);

    printf("Time taken for BFS:\t%lf\n",bfs_end-bfs_start);

    return 0;
}
