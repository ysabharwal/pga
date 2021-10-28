IDIR=include
CC=gcc
CFLAGS=-I$(IDIR)
LIBS=-fopenmp
PYTHON=python3
PREPROC=src/preprocess.py

P=4


SOURCES = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c, obj/%.o, $(SOURCES))
TARGETS = $(patsubst src/%.c, bin/%, $(SOURCES))

DEPS = $(wildcard $(IDIR)/*.h)
DATASETS = $(wildcard datasets/*.txt)
PROCESSED = $(patsubst datasets/%.txt, input/%.txt, $(DATASETS))

INPUTS = $(wildcard input/*.txt)
OUTPUT_SEQ = $(patsubst input/%.txt, output/%_seq.txt, $(INPUTS))
OUTPUT_MPI = $(patsubst input/%.txt, output/%_mpi.txt, $(INPUTS))

CMP_MPI = $(patsubst output/%_mpi.txt, %_mpi-cmp, $(OUTPUT_MPI))

.PHONY: all clean compiler linker preprocess

all: $(TARGETS)


clean:
	@rm -rf obj/*.o bin/* output/*


compile: $(OBJECTS)


linker: $(TARGETS)


preprocess: $(PROCESSED) $(PREPROC)

obj/%.o: src/%.c $(DEPS)
	@$(CC) -c $< -o $@ $(CFLAGS)

bin/%: obj/%.o
	@$(CC) $< -o $@ $(LIBS)

obj/bfs_mpi.o: src/bfs_mpi.c $(DEPS)
	@mpicc -c $< -o $@ $(CFLAGS)

bin/bfs_mpi: obj/bfs_mpi.o
	@mpicc $< -o $@

input/%.txt: datasets/%.txt
	@$(PYTHON) $(PREPROC) $< $@ 0


.PHONY: test_seq

test_seq: bin/bfs_seq $(OUTPUT_SEQ)

output/%_seq.txt: input/%.txt bin/bfs_seq
	@./run.sh seq $< $@

.PHONY: test_mpi

test_mpi: bin/bfs_mpi $(OUTPUT_MPI)

output/%_mpi.txt: input/%.txt bin/bfs_mpi
	@./run.sh mpi $< $@ $(P)

.PHONY: cmp_mpi

cmp_mpi: $(CMP_MPI)

%_mpi-cmp: output/%_mpi.txt output/%_seq.txt
	@cmp --silent $^  && (echo 'mpi output matches seq') || echo '---- FAIL mpi ----'
