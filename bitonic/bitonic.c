#include <stdio.h>      
#include <time.h>    
#include <math.h>    
#include <stdlib.h>    
#include <mpi.h>

#define MASTER 0        
#define OUTPUT_NUM 10   

void SequentialSort(void);
void CompareLow(int bit);
void CompareHigh(int bit);
int ComparisonFunc(const void* a, const void* b);
int min ( int a, int b ) { return a < b ? a : b; }
void OutputAll();


clock_t timer_start;
clock_t timer_end;
int process_rank;
int num_processes;
int* array;
int* one_before_array;
int array_size;


int main(int argc, char* argv[]) {

    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    // Store the array with one more integer in front, so we can easily transmit the size of data to be sent too
    int num_elements = atoi(argv[1]);
    array_size = num_elements / num_processes;
    array = (int*) malloc((array_size) * sizeof(int)); // For some reason there seem to be some out of array accesses

    // Generate random data in master and send out to all processes
    if(process_rank == MASTER) {
        srand(time(NULL));
        int* tmparray = (int*)malloc(num_elements * sizeof(int));
        for(int i = 0; i < num_elements; i++) {
            tmparray[i] = rand() % num_elements;
            // Already copy over own elements
            if(i < array_size)
                array[i] = tmparray[i];
        }
        for(int i = 1; i < num_processes; i++) {
            MPI_Send(
                &tmparray[i*array_size],     
                array_size,                          
                MPI_INT,                    
                i,    
                0,                          
                MPI_COMM_WORLD              
            );
        }
        free(tmparray);
    }
    else {
        MPI_Recv(
            &array[0],                       
            array_size,         
            MPI_INT,                    
            MASTER,    
            0,      
            MPI_COMM_WORLD,             
            MPI_STATUS_IGNORE           
        );
    }
    
    MPI_Barrier(MPI_COMM_WORLD); // For time measurement
    
    if (process_rank == MASTER) {
        printf("Number of Processes spawned: %d\n", num_processes);
        timer_start = clock();
    }

    
    qsort(array, array_size, sizeof(int), ComparisonFunc);


    int dimensions = (int)(log2(num_processes));    // Number of i-blocks
    for (int i = 0; i < dimensions; i++) {
        for (int j = i; j >= 0; j--) {


            // i identifies the window (blue/green box in .pdf), j the iteration within this.
            // j also represents the distance to send the data to, 0->1, 1->2, 2->4, 3->8 etc (this is used in the compare functions)
            // The actual high/low distro is magic to me
            if (((process_rank >> (i + 1)) % 2 == 0 && (process_rank >> j) % 2 == 0) || ((process_rank >> (i + 1)) % 2 != 0 && (process_rank >> j) % 2 != 0)) {
                CompareLow(j);
            } else {
                CompareHigh(j);
            }
        }
    }

    
    MPI_Barrier(MPI_COMM_WORLD); // Replace with MPI-Gather if you actually want to distribute data

    if (process_rank == MASTER) {
        timer_end = clock();
        printf("Time Elapsed (Sec): %f\n", (double)(timer_end - timer_start) / CLOCKS_PER_SEC);
    }
    //OutputAll();

    free(array);

    
    MPI_Finalize();
    return 0;
}

void OutputAll() {
    for(int i = 0; i < num_processes; i++) {
        MPI_Barrier(MPI_COMM_WORLD);
        if (i == process_rank) {
            printf("Process %d: ", i);
            for (int j = 0; j < min(OUTPUT_NUM, array_size); j++) {
                printf("%d ",array[j]);
            }
            printf("\n");
        }
    }
}

int ComparisonFunc(const void* a, const void* b) {
    return ( *(int*)a - *(int*)b );
}

// This one is a low-element
void CompareLow(int j) {
    // Send everything to the other process, that one sorts it and sends it back
    // TODO optimize by sending only what's between the medians 
    // the high party could send it's median, 
    //   if their median is higher, send upper half and receive their lower half
    //   if our median is higher, also send all array elements below the middle that are still higher than their median and wait for them to send back their lower half afterwards
    MPI_Send(
        &array[0],     
        array_size,                          
        MPI_INT,                    
        process_rank ^ (1 << j),    
        0,                          
        MPI_COMM_WORLD              
    );
    

    MPI_Recv(
        &array[0],                       
        array_size,         
        MPI_INT,                    
        process_rank ^ (1 << j),    
        0,                          
        MPI_COMM_WORLD,             
        MPI_STATUS_IGNORE           
    );

    return;
}



void CompareHigh(int j) {
    
    
    // Receive the data from the other partner, sort it, send it back
    int* buffer = (int*)malloc(array_size * 2 * sizeof(int));
    MPI_Recv(
        &buffer[0],                       
        array_size*2,         
        MPI_INT,                    
        process_rank ^ (1 << j),    
        0,                          
        MPI_COMM_WORLD,             
        MPI_STATUS_IGNORE           
    );

    // Copy over our part of the array
    for(int i=0; i< array_size; i++) {
        buffer[array_size + i] = array[i];
    }

    // Sort everything
    qsort(buffer, array_size*2, sizeof(int), ComparisonFunc);

    MPI_Send(
        &buffer[0],     
        array_size,                          
        MPI_INT,                    
        process_rank ^ (1 << j),    
        0,                          
        MPI_COMM_WORLD              
    );

    // Copy back the buffer
    for(int i=0; i<array_size; i++) {
        array[i] = buffer[i + array_size];
    }

    free(buffer);
    return;
}

