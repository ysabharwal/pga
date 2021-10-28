if [[ $1 = seq ]]
then
	gcc src/dijkstra_seq.c -w -o0 -o bin/dijkstra_seq
elif [[ $1 = omp ]]
then
	gcc src/dijkstra_omp.c -w -o0 -o bin/dijkstra_omp -fopenmp
fi

