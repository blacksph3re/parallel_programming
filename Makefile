compile = g++ -std=c++11 -lboost_program_options -O3
testdata_mult = data/1500x1500.matrix
testdata_mpi = data/18000x18000.matrix
testdata_add = data/10000x10000.matrix
testdata_gauss = data/2500x2500.matrix data/2500x1.matrix
.PHONY: all build clean data


##################################
# Additions

build:
	mkdir -p build
	mkdir -p timing
	mkdir -p data
	$(compile) -o build/genData genData.cpp
	make add
	make mult
	make gauss

clean:
	rm -f build/*
	rm -f data/*
	rm -f timing/*

data:
	rm -f data/*
	echo "----- Generating data --------"
	ROWS=5 COLS=1 GENINT=true build/genData
	ROWS=5 COLS=5 GENINT=true build/genData
	ROWS=1500 COLS=1500 build/genData
	ROWS=2500 COLS=1 build/genData
	ROWS=2500 COLS=2500 build/genData
	ROWS=10000 COLS=10000 build/genData
	ROWS=18000 COLS=18000 build/genData

runall:
	rm -f timing/*
	make data
	make runadd
	make runmult
	make rungauss

##################################
# Additions

runadd:
	echo "----- Running addition benchmarks --------"
	make addSeq
	make addThreads
	make addOmp
	make addMpi

add:
	$(compile) -o build/addSeq addSeq.cpp
	$(compile) -pthread -o build/addThreads addThreads.cpp
	$(compile) -pthread -fopenmp -o build/addOmp addOmp.cpp
	mpic++ -std=c++11 -lboost_program_options -O3 -o build/addMpi addMpi.cpp


addSeq:
	build/addSeq --timing_output=timing/addSeq --file_out=/dev/null $(testdata_add)

addOmp:
	OMP_NUM_THREADS=1 build/addOmp --timing_output=timing/addOmp --file_out=/dev/null $(testdata_add)
	OMP_NUM_THREADS=2 build/addOmp --timing_output=timing/addOmp --file_out=/dev/null $(testdata_add)
	OMP_NUM_THREADS=4 build/addOmp --timing_output=timing/addOmp --file_out=/dev/null $(testdata_add)
	OMP_NUM_THREADS=8 build/addOmp --timing_output=timing/addOmp --file_out=/dev/null $(testdata_add)
	OMP_NUM_THREADS=16 build/addOmp --timing_output=timing/addOmp --file_out=/dev/null $(testdata_add)

addThreads:
	build/addThreads --n=1 --timing_output=timing/addThreads --file_out=/dev/null $(testdata_add)
	build/addThreads --n=2 --timing_output=timing/addThreads --file_out=/dev/null $(testdata_add)
	build/addThreads --n=4 --timing_output=timing/addThreads --file_out=/dev/null $(testdata_add)
	build/addThreads --n=8 --timing_output=timing/addThreads --file_out=/dev/null $(testdata_add)
	build/addThreads --n=16 --timing_output=timing/addThreads --file_out=/dev/null $(testdata_add)

addMpi:
	mpiexec -np 1 build/addMpi --timing_output=timing/addMpi --file_out=/dev/null $(testdata_add)
	mpiexec -np 2 build/addMpi --timing_output=timing/addMpi --file_out=/dev/null $(testdata_add)
	mpiexec -np 4 build/addMpi --timing_output=timing/addMpi --file_out=/dev/null $(testdata_add)
	mpiexec -np 8 build/addMpi --timing_output=timing/addMpi --file_out=/dev/null $(testdata_add)
	mpiexec -np 16 build/addMpi --timing_output=timing/addMpi --file_out=/dev/null $(testdata_add)

###################################
# Multiplications

runmult:
	echo "----- Running multiplication benchmarks --------"
	make multSeq
	make multThreads
	make multOmp

mult:
	$(compile) -o build/multSeq multSeq.cpp
	$(compile) -pthread -o build/multThreads multThreads.cpp
	$(compile) -pthread -fopenmp -o build/multOmp multOmp.cpp

multSeq:
	build/multSeq --timing_output=timing/multSeq --file_out=/dev/null $(testdata_mult)

multThreads:
	build/multThreads --n=1 --timing_output=timing/multThreads --file_out=/dev/null $(testdata_mult)
	build/multThreads --n=2 --timing_output=timing/multThreads --file_out=/dev/null $(testdata_mult)
	build/multThreads --n=4 --timing_output=timing/multThreads --file_out=/dev/null $(testdata_mult)
	build/multThreads --n=8 --timing_output=timing/multThreads --file_out=/dev/null $(testdata_mult)
	build/multThreads --n=16 --timing_output=timing/multThreads --file_out=/dev/null $(testdata_mult)

multOmp:
	OMP_NUM_THREADS=1 build/multOmp --timing_output=timing/multOmp --file_out=/dev/null $(testdata_mult) 
	OMP_NUM_THREADS=2 build/multOmp --timing_output=timing/multOmp --file_out=/dev/null $(testdata_mult) 
	OMP_NUM_THREADS=4 build/multOmp --timing_output=timing/multOmp --file_out=/dev/null $(testdata_mult) 
	OMP_NUM_THREADS=8 build/multOmp --timing_output=timing/multOmp --file_out=/dev/null $(testdata_mult) 
	OMP_NUM_THREADS=16 build/multOmp --timing_output=timing/multOmp --file_out=/dev/null $(testdata_mult) 

###################################
# Gaussian Elimination

rungauss:
	echo "----- Running gaussian elimination benchmarks --------"
	make gaussSeq
	make gaussOmp

gauss:
	$(compile) -o build/gaussSeq gaussSeq.cpp
	$(compile) -pthread -fopenmp -o build/gaussOmp gaussOmp.cpp

gaussSeq:
	build/gaussSeq --timing_output=timing/gaussSeq --file_out=/dev/null $(testdata_gauss)

gaussOmp:
	OMP_NUM_THREADS=1 build/gaussOmp --timing_output=timing/gaussOmp --file_out=/dev/null $(testdata_gauss)
	OMP_NUM_THREADS=2 build/gaussOmp --timing_output=timing/gaussOmp --file_out=/dev/null $(testdata_gauss)
	OMP_NUM_THREADS=4 build/gaussOmp --timing_output=timing/gaussOmp --file_out=/dev/null $(testdata_gauss)
	OMP_NUM_THREADS=8 build/gaussOmp --timing_output=timing/gaussOmp --file_out=/dev/null $(testdata_gauss)
	OMP_NUM_THREADS=16 build/gaussOmp --timing_output=timing/gaussOmp --file_out=/dev/null $(testdata_gauss)
