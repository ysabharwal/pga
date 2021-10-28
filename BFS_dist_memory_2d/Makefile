SRC=src/bfs_serial.c
APP=bin/run.out
MPI_PROG=bin/mpi_run.out
MPI_SRC=src/bfs_2d_mpi.c
CC=gcc
MPICC=mpicc
C_FLAGS= -I include/ctl

all: $(MPI_PROG)

serial: $(APP)

$(APP): $(SRC)
	@$(CC) -Wall -O0 $(SRC) $(C_FLAGS) -o $(APP)

$(MPI_PROG): $(MPI_SRC)
	@$(MPICC) -Wall -O0 $(MPI_SRC) $(C_FLAGS) -o $(MPI_PROG)
