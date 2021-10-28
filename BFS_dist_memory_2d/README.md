# BFS in 2D processor space

BFS for distributed memory with 2D partitioning using MPI

## Running the program

To run the program, execute

```bash
./exec.sh <input_file> <output_file> <R> <C> <UNDIRECTED>
```

R and C are the number of processor rows and columns respectively. Undirected should be 1 if the graph is undirected and 0 otherwise.

To run the serial program for comparison, execute

```bash
./exec_serial.sh <input_file> <output_file> <UNDIRECTED>
```

## Working of the program

- The python script converts the edge file given as input to an appropriate format while redistributing the edges so that the distribution of edges becomes more or less uniform.
- The program takes the files created by the python script as input, does BFS on them in either serial manner or in parallel with 2D processor space using MPI.
- The program reports the time taken to do just BFS on the entire graph. For now, the time taken to get the input is excluded to showcase the timings of the actual parallel program.
- The program outputs the levels of nodes assuming we take the node that is first mentioned in the original edge file as the root. For most files, this will be the node marked as 0. This behaviour can be changed in the python script.