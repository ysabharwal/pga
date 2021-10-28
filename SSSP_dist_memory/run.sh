if [[ $1 = seq ]]
then
	./bin/dijkstra_seq $2 $3
elif [[ $1 = omp ]]
then
	./bin/dijkstra_omp $2 $3 $4
fi