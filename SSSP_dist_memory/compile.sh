if [[ $1 = seq ]]
then
	gcc src/dijkstra_seq.c -w -o0 -o bin/dijkstra_seq
elif [[ $1 = mpi ]]
then
	mpicc -g -Wall -w -o bin/dijkstra_mpi src/dijkstra_mpi.c
fi

