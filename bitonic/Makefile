

build:
	mpicc -lm -g -o bitonic bitonic.c

run:
	mpirun -n 4 bitonic 28

longrun:
	mpirun -n 4 bitonic 100000000

debug:
	mpirun -n 1 gdb bitonic

scalingtest:
	mpirun -n 1 bitonic 10000000
	mpirun -n 2 bitonic 10000000
	mpirun -n 4 bitonic 10000000
	mpirun -n 8 bitonic 10000000
	mpirun -n 16 bitonic 10000000