

build:
	mpicc -lm -g -o bitonic -O3 bitonic.c
	gcc -o quicksort -O3 quicksort.c

quicksort:
	./quicksort 50000000

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
	mpirun -n 32 bitonic 10000000
	mpirun -n 64 bitonic 10000000