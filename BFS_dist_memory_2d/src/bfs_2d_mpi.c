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

#include <mpi.h>
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

int max_int(int a, int b) {
    return (a>b) ? a : b;
}

//Data for neighbours is stored in columns
//So size of neighbours is col_end-col_start
typedef struct {
    int coords[2];
    int processor_dims[2];
    int rank;
    int n;
    int m;
    int source;
    int undirected;
    int num_edges;
    int row_start;
    int row_end;
    int col_start;
    int col_end;
    lst_int *neighbours;
    int level_start;
    int level_end;
    vec_int levels;
    vec_int rev_mapping;
} process_data;

int prow(int u, int dims[], int n) {
    int i=0;
    while((i<dims[0]) && ((((i+1)*n)/dims[0]) <= u) )
        i++;
    return i;
}

int pcol(int u, int dims[], int n) {
    int i=0;
    while((i<dims[1]) && ((((i+1)*n)/dims[1]) <= u) )
        i++;
    return i;
}

#define belongs(DATA,__U,__V) ((__U)>=DATA->col_start) && ((__U)<DATA->col_end) && ((__V)>=DATA->row_start) && ((__V)<DATA->row_end)
#define is_owner(DATA,__U) ((__U)>=DATA->level_start) && ((__U)<DATA->level_end)
#define level_index(DATA,__U) ((__U)-DATA->level_start)
#define row_index(DATA,__U) ((__U)-DATA->row_start)
#define col_index(DATA,__U) ((__U)-DATA->col_start)
#define RANK(PROW,PCOL,DATA) (PROW*DATA->processor_dims[1]+PCOL)
#define PROW(DATA,__U) prow(__U, DATA->processor_dims, DATA->n)
#define PCOL(DATA,__U) pcol(__U, DATA->processor_dims, DATA->n)

int compare(int* a, int* b) { return *a == *b ? 0 : *a < *b ? -1 : 1; }


process_data* read_input(char* filename, char* rev_mapping_file, int coords[], int dims[], int rank) {
    process_data *data = (process_data*)malloc(sizeof(process_data));
    data->coords[0] = coords[0];
    data->coords[1] = coords[1];
    data->processor_dims[0] = dims[0];
    data->processor_dims[1] = dims[1];
    data->rank = rank;

    //Read file
    FILE *f = fopen(filename, "r");

    if(!f) {
        printf("Reading file %s failed\n", filename);
        exit(0);
    }

    int n,m,source,undirected;

    fscanf(f, "%d\t%d\t%d\t%d\n", &n, &m, &source, &undirected);

    data->n = n;
    data->m = m;
    data->source = source;
    data->undirected = undirected;

    data->row_start = (n*coords[0])/dims[0];
    data->row_end = (n*(coords[0]+1))/dims[0];

    data->col_start = (n*coords[1])/dims[1];
    data->col_end = (n*(coords[1]+1))/dims[1];

    data->neighbours = (lst_int*)malloc((data->col_end-data->col_start)*sizeof(lst_int));
    for(int i=0; i<data->col_end-data->col_start; i++) {
        data->neighbours[i] = lst_int_init();
    }

    int i=0;
    int u,v,uv;
    int index;
    int num_edges = 0;
    while( fscanf(f,"%d\t%d\t%d\n",&u,&v,&uv) && (i<m) ) {
        if(undirected) {
            if( belongs(data,u,v) ) {
                index = col_index(data,u);
                lst_int_push_back(&data->neighbours[index],v);
                num_edges++;
            }
            if( belongs(data,v,u) ) {
                index = col_index(data,v);
                lst_int_push_back(&data->neighbours[index],u);
                num_edges++;
            }
        }
        else {
            if( belongs(data,u,v) ) {
                index = col_index(data,u);
                lst_int_push_back(&data->neighbours[index],v);
                num_edges++;
            }
        }
        i++;
    }
    fclose(f);
    data->num_edges = num_edges;

    int has_started = 0;
    data->level_start = -1;
    data->level_end = -1;
    data->levels = vec_int_init();
    for(i=data->col_start; i<data->col_end; i++) {
        if( belongs(data,i,i) ) {
            if(!has_started) {
                has_started = 1;
                data->level_start = i;
            }
            vec_int_push_back(&data->levels,-1);
        }
        else {
            if(has_started) {
                data->level_end = i;
                has_started = 0;
                break;
            }
        }
    }
    if(has_started) {
        data->level_end = data->col_end;
        has_started = 0;
    }

    data->rev_mapping = vec_int_init();
    if(data->level_start!=-1) {
        f = fopen(rev_mapping_file, "r");
        int tempn,u,v;
        fscanf(f,"%d\n",&tempn);
        for(int i=0; i<data->level_start; i++) {
            fscanf(f,"%d\t%d\n",&u,&v);
        }
        for(int i=data->level_start; i<data->level_end; i++) {
            fscanf(f,"%d\t%d\n",&u,&v);
            vec_int_push_back(&data->rev_mapping, v);
        }
        fclose(f);
    }

    return data;
}

void parallel_BFS(process_data *data, MPI_Comm cart_row, MPI_Comm cart_col) {
    //Do BFS
    //Define some common variables here
    //int myprow = data->coords[0];
    int mypcol = data->coords[1];
    int curr_level = 0;
    int total_vertices_covered = 1;
    //int completion_tag = 1;

    set_int frontier = set_int_init(compare);
    set_int Recv_frontier = set_int_init(compare);
    set_int Recv_next_frontier = set_int_init(compare);
    set_int next_frontier[data->processor_dims[1]];
    for(int i=0; i<data->processor_dims[1]; i++)
        next_frontier[i] = set_int_init(compare);


    if(is_owner(data,data->source)) {
        set_int_insert(&frontier, data->source);
        data->levels.value[level_index(data,data->source)] = 0;
    }

    //To simplify transfer of data and so that 2 processors don't wait on each other, the lower ranked processor sends first
    while(1) {
        //Send vertices through the column
        //These processors contain neighbours for those nodes
        //But the one broadcasting will mostly be the processors who own the nodes for this column
        //Make recv frontier empty
        set_int_clear(&Recv_frontier);
        for(int q=0; q<data->processor_dims[0]; q++) {
            int d = RANK(q,mypcol,data);
            int s;

            if(d==data->rank)
                s = frontier.size;
            MPI_Bcast(&s, 1, MPI_INT, q, cart_col);
            if(s==0) {
                //Do nothing
            }
            else {
                int *Rfrontier = malloc(s*sizeof(int));
                if(d==data->rank) {
                    int i=0;
                    foreach(set_int, &frontier, it) {
                        Rfrontier[i] = *it.ref;
                        i++;
                    }
                }
                MPI_Bcast(Rfrontier, s, MPI_INT, q, cart_col);

                //Combine into Recv_frontier
                for(int i=0; i<s; i++) {
                    set_int_insert(&Recv_frontier, Rfrontier[i]);
                }

                free(Rfrontier);
            }
        }

        //Form the next frontier locally
        for(int i=0; i<data->processor_dims[1]; i++)
            set_int_clear(&next_frontier[i]);
        foreach(set_int, &Recv_frontier, it) {
            int u = *it.ref;
            int l_index = level_index(data,u);
            int n_index = col_index(data,u);
            //Unnecessary
            if(is_owner(data,u))
                data->levels.value[l_index] = curr_level;
            if(PCOL(data,u)!=mypcol)
                printf("Not possible\n");
            foreach(lst_int, &data->neighbours[n_index], it2) {
                int v = *it2.ref;
                if(v==-1)
                    printf("what?\n");
                int dcol = PCOL(data,v);
                set_int_insert(&next_frontier[dcol], v);
            }
        }

        set_int_clear(&Recv_next_frontier);
        //Send next frontier along rows

        {
            int max_next_frontier_size = 0;
            int max_next_frontier_size_combined;
            int send_nums[data->processor_dims[1]];
            for(int i=0; i<data->processor_dims[1]; i++) {
                max_next_frontier_size = max_int(max_next_frontier_size, next_frontier[i].size);
                send_nums[i] = next_frontier[i].size;
            }


            MPI_Allreduce(&max_next_frontier_size, &max_next_frontier_size_combined, 1, MPI_INT, MPI_MAX, cart_row);
            if(max_next_frontier_size_combined==0) {
                //Do nothing
            }
            else {
                int recv_nums[data->processor_dims[1]];
                MPI_Alltoall(send_nums, 1, MPI_INT, recv_nums, 1, MPI_INT, cart_row);

                int send_size[data->processor_dims[1]];
                int recv_size[data->processor_dims[1]];
                int send_disp[data->processor_dims[1]];
                int recv_disp[data->processor_dims[1]];

                int send_buffer_size = 0;
                int recv_buffer_size = 0;

                for(int i=0; i<data->processor_dims[1]; i++) {
                    send_size[i] = send_nums[i]>0 ? send_nums[i] : 1;
                    recv_size[i] = recv_nums[i]>0 ? recv_nums[i] : 1;
                    send_disp[i] = send_buffer_size;
                    recv_disp[i] = recv_buffer_size;
                    send_buffer_size += send_size[i];
                    recv_buffer_size += recv_size[i];
                }

                int *Sbuffer = (int*)malloc(send_buffer_size*sizeof(int));
                int *Rbuffer = (int*)malloc(recv_buffer_size*sizeof(int));

                for(int i=0; i<data->processor_dims[1]; i++) {
                    int j = send_disp[i];
                    if(send_nums[i]==0)
                        Sbuffer[j] = -1;
                    else {
                        foreach(set_int, &next_frontier[i], it) {
                            Sbuffer[j] = *it.ref;
                            j++;
                        }
                    }
                }

                MPI_Alltoallv(Sbuffer, send_size, send_disp, MPI_INT,
                                Rbuffer, recv_size, recv_disp, MPI_INT,
                                cart_row  
                                );
                
                for(int i=0; i<data->processor_dims[1]; i++) {
                    if(recv_nums[i]==0) {
                        //Wasn't supposed to receive anything meaningful
                    }
                    else {
                        for(int j=recv_disp[i]; j<recv_disp[i]+recv_size[i]; j++) {
                            set_int_insert(&Recv_next_frontier, Rbuffer[j]);
                        }
                    }
                }

                free(Rbuffer);
                free(Sbuffer);


            }

        }

        //Combine values into next frontier === frontier
        set_int_clear(&frontier);

        foreach(set_int, &Recv_next_frontier, it) {
            int u = *it.ref;
            if(is_owner(data,u)) {
                int l_index = level_index(data,u);
                if(data->levels.value[l_index]==-1) {
                    data->levels.value[l_index] = curr_level+1;
                    set_int_insert(&frontier, u);
                }
            }
        }



        curr_level++;
        int size = frontier.size;
        int total_size;
        MPI_Allreduce(&size, &total_size, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        total_vertices_covered += total_size;

        //printf("%d\t%d\t%d\t%d\t%d\t%d\n", curr_level, coords[0], coords[1], size, total_size, total_vertices_covered);

        if(total_size==0) {
            break;
        }

    }
}


int main(int argc, char **argv) {
    if(argc!=6) {
        printf("5 inputs needed\n");
        printf("> input graph file name\n");
        printf("> Number of rows of processors\n");
        printf("> Number of cols of processors\n");
        printf("> reverse mapping file name\n");
        printf("> Output file name\n");
        return -1;
    }
    char *filename = argv[1];
    int rows_processor = atoi(argv[2]);
    int cols_processor = atoi(argv[3]);
    char *rev_mapping_file = argv[4];
    char *output_file = argv[5];

    //For the 2D cartesian topology of the processors
    int numtasks, rank;
    int ndim = 2;
    int dims[2] = {rows_processor, cols_processor};
    int periods[2] = {1,1};
    //reorder allows the topology to be changed later on
    int reorder = 0;
    int coords[2];
    MPI_Comm cartcomm;
    int SIZE = rows_processor*cols_processor;

    //Test run
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    if (numtasks == SIZE) {
        // create cartesian virtual topology, get rank, coordinates, neighbor ranks
        MPI_Cart_create(MPI_COMM_WORLD, ndim, dims, periods, reorder, &cartcomm);
        MPI_Comm_rank(cartcomm, &rank);
        MPI_Cart_coords(cartcomm, rank, 2, coords);

        MPI_Comm cart_row, cart_col;
        int remaindims_row[2] = {0,1};
        int remaindims_col[2] = {1,0};

        MPI_Cart_sub(cartcomm, remaindims_row, &cart_row);
        MPI_Cart_sub(cartcomm, remaindims_col, &cart_col);

        //For each processor, read the input

        //double start,finish;
        //start = MPI_Wtime();
        process_data *data = read_input(filename, rev_mapping_file, coords, dims, rank);
        //finish = MPI_Wtime();
        //printf("%d\t%d\t%d\t%d\t%d\t%d\t%lf\n", rank, coords[0], coords[1], data->num_edges, data->level_start, data->level_end, finish-start);

        MPI_Barrier(MPI_COMM_WORLD);

        double bfs_start, bfs_end;
        if(data->rank==0) {
            bfs_start = MPI_Wtime();
            //bfs_start = get_wall_time();
        }


        parallel_BFS(data, cart_row, cart_col);

        MPI_Barrier(MPI_COMM_WORLD);

        if(data->rank==0) {
            bfs_end = MPI_Wtime();
            //bfs_end = get_wall_time();
        }
        
        for(int i=0; i<data->processor_dims[0]*data->processor_dims[1]; i++) {
            if(i==data->rank) {
                if(data->level_start!=-1) {
                    FILE *f;
                    if(i!=0)
                        f = fopen(output_file, "a");
                    else
                        f = fopen(output_file, "w");
                    for(int j=data->level_start; j<data->level_end; j++) {
                        int index = j-data->level_start;
                        fprintf(f,"%d\t%d\n",data->rev_mapping.value[index],data->levels.value[index]);
                    }
                    fclose(f);
                }
            }

            MPI_Barrier(MPI_COMM_WORLD);
        }

        if(data->rank==0)
            printf("Time taken for BFS:\t%lf\n",bfs_end-bfs_start);
    }
    else
        printf("Must specify %d processors. Terminating.\n",SIZE);

    MPI_Finalize();

    return 0;
}




