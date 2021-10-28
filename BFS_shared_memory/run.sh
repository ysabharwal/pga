#!/usr/bin/bash

if [[ $1 = seq ]]
then
  ./bin/bfs_seq $2 | tee -i $3
elif [[ $1 = omp ]]
then
  ./bin/bfs_omp $2 $4 | tee -i $3
elif [[ $1 = preprocess ]]
then
  python3 src/preprocess.py $2 $3 $4
fi
