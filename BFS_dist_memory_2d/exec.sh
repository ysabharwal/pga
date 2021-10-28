if [ $# -ne 5 ]; then
    echo "Need 5 parameters";
    echo "> Input file name";
    echo "> Output file name";
    echo "> Number of processors rows";
    echo "> Number of processors columns";
    echo "> Is the graph undirected(1 if yes else 0)"
else
    FILENAME=$1
    OUTPUT=$2
    R=$3
    C=$4
    UNDIRECTED=$5
    TEMP_EDGE_FILE=temp.edge.txt
    TEMP_REV_MAP_FILE=temp.rev_map.txt
    TEMP_OUTPUT_FILE=temp.out.txt
    MODIFY_SCRIPT=scripts/modify_edge_file.py
    MPI_EXEC=bin/mpi_run.out
    EXEC=bin/run.out

    echo "Creating temporary edge file" &&
    python3 $MODIFY_SCRIPT $FILENAME $TEMP_EDGE_FILE $TEMP_REV_MAP_FILE $UNDIRECTED &&
    echo "Compiling files" &&
    make &&
    echo "Running program" &&
    mpiexec --oversubscribe -n $(($R * $C)) $MPI_EXEC $TEMP_EDGE_FILE $R $C $TEMP_REV_MAP_FILE $TEMP_OUTPUT_FILE &&
    echo "Program executed. Outputting results to output file" &&
    sort -n $TEMP_OUTPUT_FILE > $OUTPUT 
    rm -f $TEMP_EDGE_FILE $TEMP_OUTPUT_FILE $TEMP_REV_MAP_FILE $MPI_EXEC $EXEC &&
    echo "Execution completed"
fi
