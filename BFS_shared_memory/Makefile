IDIR=include
CC=gcc
CFLAGS=-I$(IDIR)
LIBS=-fopenmp
PYTHON=python3
PREPROC=src/preprocess.py

T=8


SOURCES = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c, obj/%.o, $(SOURCES))
TARGETS = $(patsubst src/%.c, bin/%, $(SOURCES))

DEPS = $(wildcard $(IDIR)/*.h)
DATASETS = $(wildcard datasets/*.txt)
PROCESSED = $(patsubst datasets/%.txt, input/%.txt, $(DATASETS))

INPUTS = $(wildcard input/*.txt)
OUTPUT_SEQ = $(patsubst input/%.txt, output/%_seq.txt, $(INPUTS))
OUTPUT_OMP = $(patsubst input/%.txt, output/%_omp.txt, $(INPUTS))

CMP_OMP = $(patsubst output/%_omp.txt, %_omp-cmp, $(OUTPUT_OMP))

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

input/%.txt: datasets/%.txt
	@$(PYTHON) $(PREPROC) $< $@ 0


.PHONY: test_seq

test_seq: bin/bfs_seq $(OUTPUT_SEQ)

output/%_seq.txt: input/%.txt bin/bfs_seq
	@./run.sh seq $< $@

.PHONY: test_omp

test_omp: bin/bfs_omp $(OUTPUT_OMP)

output/%_omp.txt: input/%.txt bin/bfs_omp
	@./run.sh omp $< $@ $(T)

.PHONY: cmp_omp

cmp_omp: $(CMP_OMP)

%_omp-cmp: output/%_omp.txt output/%_seq.txt
	@cmp --silent $^  && (echo 'omp output matches seq') || echo '---- FAIL omp ----'
