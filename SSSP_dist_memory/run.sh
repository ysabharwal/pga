if [[ $1 = seq ]]
then
	./bin/dijkstra_seq $2 $3
elif [[ $1 = mpi ]]
then
	# lamboot
	mpiexec -n $4 ./bin/dijkstra_mpi $2 $3 $4
fi