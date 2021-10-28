inp = input/input.txt
out_seq = output/output_seq.txt
out_omp = output/output_omp.txt

compile_all: bin/dijkstra_seq bin/dijkstra_omp

bin/dijkstra_seq: src/dijkstra_seq.c include/util.h
	bash compile.sh seq
bin/dijkstra_omp: src/dijkstra_omp.c include/util.h
	bash compile.sh omp

run_seq: bin/dijkstra_seq
	bash run.sh seq $(inp) $(out_seq)
run_omp: bin/dijkstra_omp
	bash run.sh omp $(inp) $(out_omp) 8

# Testing -----------------------------------------------------------------------------------------
# Usage: Run `make test t=5` for testing on 5 randomly generated graphs

testing/generate_testcase: testing/generate_testcase.cpp
	g++ -std=c++17 -o testing/generate_testcase testing/generate_testcase.cpp
gen_tc:
	@./testing/generate_testcase input/random_tc.txt 1 # 1 => undirected

test:
	@echo Testing on $(t) random test cases ...
	@tc=$(t); \
		while [ $${tc} -gt 0 ] ; do \
			make --no-print-directory test_testcase TESTCASE=`expr $$t - $$tc + 1`; \
			tc=`expr $$tc - 1`; \
		done; \
		true

test_testcase: compile_all testing/generate_testcase
	@echo Test Case: $(TESTCASE) --------
	@echo '    Generating Graph'
	@./testing/generate_testcase testing/auto_gen_tcs/tc_$(TESTCASE).txt 1
	@echo -n '    seq:'
	@bash run.sh seq testing/auto_gen_tcs/tc_$(TESTCASE).txt $(out_seq)
	@echo -n '    omp: '
	@bash run.sh omp testing/auto_gen_tcs/tc_$(TESTCASE).txt $(out_omp) 4
	@cmp --silent $(out_seq) $(out_omp)  && (echo '    OMP Pass') || echo '    OMP FAIL ***************'

clean_tcs:
	@rm -f testing/generate_testcase
	@rm -f testing/auto_gen_tcs/*.txt

# Data Processing ---------------------------------------------------------------------------------
preprocess:
	python data/modify_edge_file.py $(ZIPFILE_NAME)  # #ZIPFILENAME must exist in data/compressed

# Cleaning ----------------------------------------------------------------------------------------

clean:
	rm -f bin/*

clear_output:
	rm -f output/*
