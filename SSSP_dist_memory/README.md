# Shared Memory SSSP using OMP

**Author: Satwik Banchhor (2018CS10385)**

Source code, implementing djikstra's algorithm in sequential and shared memory, can be found in the `src/` directory:

```
.
└── src
    ├── dijkstra_omp.c     /* Shared memory implementation using OMP */
    └── dijkstra_seq.c     /* Sequential Implemenation */
```

Processed data for selected snap datasets can be found in the `data/processed/` directory:

```
.
└── data
    └── processed
        ├── musae_git_edges.txt
        ├── processed_email-Enron.txt
        ├── processed_loc-brightkite_edges.txt
        ├── processed_p2p-Gnutella30.txt
        └── processed_sx-mathoverflow.txt
```

### Useful commands

#### Compiling:

To compile both the implementations execute: `make`

Individual implementations can be compiled using the following commands:

```
bash compile.sh seq
bash compile.sh omp
```

#### Running:

Individual files can be run using any of the following commands:

```
bash run.sh seq $(input_filename) $(output_filename)
bash run.sh omp $(input_filename) $(output_filename) $(number of threads)
```

Note: Input file must be in the format given below.

#### Pre-processing:

For processing `input_file` (raw) to `output_file` (processed input):

```
python data/modify_edge_file.py $(input_file) $(output_file)
```

The pre-processing script works on raw datasets with the following assumptions:

1. Lines beginning with 2 numeric tokens represent an unweighted edge: `u v` such edges are assigned weights 1
3. Other lines are ignored

**Note**: To save space the edge weights are stored as `short int`, no appropriate "weighted" snap datasets were found for which the implementations could be tested, hence, for the snap datasets all the edges were assigned unit weights. More extensive testing was done on randomly weighted graphs (discussed later).

Processed file format which can be given as input to the dijkstra's implementations:

```
n m r undir
u v w_uv
u v w_uv
.
.
.
u v w_uv
```

where:

1. `n`: Number of edges
2. `m`: Number of vertices
3. `r`: Source node (root) from which we want to find distances
4. `undir`: Boolean value which tells whether graph is directed or undirected
5. Next m lines contain information for individual edges: `u v w_uv` which tells that there is an edge between `u` and `v` (for undirected graph) or from `u` to `v` (for directed graph) of weight `w_uv`.

### Results

Since this implementation is for dense graphs we use adjacency matrix to store the graph input. Hence $O(\#Nodes^2)$ space (overall) is required for storing input.

| **Data set**                                                 | **# Nodes** | **Speedup OMP (t = 4)** | **Speedup OMP (t = 8)** |
| ------------------------------------------------------------ | ----------- | ----------------------- | ----------------------- |
| [p2p-Gnutella30](https://snap.stanford.edu/data/p2p-Gnutella30.html) | 36,682      | 2.118830182             | 2.373851603             |
| [sx-mathoverflow](https://snap.stanford.edu/data/sx-mathoverflow.html) | 24,818      | 1.649392791             | 2.098633524             |
| [email-Enron](https://snap.stanford.edu/data/email-Enron.html) | 36,692      | 2.012629107             | 1.670197533             |
| [musae-github](https://snap.stanford.edu/data/github-social.html) | 37,700      | 1.610152693             | 1.848421818             |

On my system, I was not able to allocate more than 6 GB of data. The smallest dataset for which I was not able to allocate memory for the adjacency matrix was: **[loc-Brightkite](https://snap.stanford.edu/data/loc-Brightkite.html) (58,228 nodes)**.

#### Testing on random weighted graphs

For testing, I created a script that generates a random weighted graph and tests the OMP and MPI outputs against the sequential output.

Command: `make test t=$(number of random tests)`

The above command tests the implementations on `t` randomly generated graphs, all the testcases are stored in the `testing/auto_gen_tcs/` directory.

### Design details

Pseudocode  for dijkstra's algorithm:

```pseudocode
SSSP_seq(V, E, W, r) {
	T = {r};
    d[r] = 0;
    for all v in V \ T:
    	 if (r, v) in E: d[v] = w(r, v)
         else : d[v] = inf
    
    for n - 1 iterations:
    	find u st d[u] = min{d[v] | v in V \ T} // find min operation
    	T = T union {u}
    	for all v in V \ T:
 		   	d[v] = min{d[v], d[u] + w[u, v]}    // update operation (relaxation)
    	
    return d;
}
```

We use adjacency matrix $W_{|V| \times |V|}$ for representing the graph $G(V, E, W)$.

#### OMP Implementation

We use the following parallelization strategies:

1. Find min operation is done using 2 omp min reductions: first to find the weight of the (new) minimum distance vertex $u$, then to find the index $u$ (= vertex id) of the vertex with the minimum weight.
2. Relaxation is done parallelly using parallel for.