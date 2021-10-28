if [ $# -ne 3 ]; then
    echo "Need 3 parameters";
    echo "> Input file name";
    echo "> Output file name";
    echo "> Is the graph undirected(1 if yes else 0)"
else
    FILENAME=$1
    OUTPUT=$2
    UNDIRECTED=$3
    TEMP_EDGE_FILE=temp.serial.edge.txt
    TEMP_REV_MAP_FILE=temp.serial.rev_map.txt
    TEMP_OUTPUT_FILE=temp.serial.out.txt
    MODIFY_SCRIPT=scripts/modify_edge_file.py
    MPI_EXEC=bin/mpi_run.out
    EXEC=bin/run.out

    echo "Creating temporary edge file" &&
    python3 $MODIFY_SCRIPT $FILENAME $TEMP_EDGE_FILE $TEMP_REV_MAP_FILE $UNDIRECTED &&
    echo "Compiling files" &&
    make serial &&
    echo "Running program" &&
    ./$EXEC $TEMP_EDGE_FILE $TEMP_REV_MAP_FILE $TEMP_OUTPUT_FILE &&
    echo "Program executed. Outputting results to output file" &&
    sort -n $TEMP_OUTPUT_FILE > $OUTPUT 
    rm -f $TEMP_EDGE_FILE $TEMP_OUTPUT_FILE $TEMP_REV_MAP_FILE $MPI_EXEC $EXEC &&
    echo "Execution completed"
fi
