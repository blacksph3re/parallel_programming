#include <time.h>
#include <stdio.h>
#include <stdlib.h>


int ComparisonFunc(const void* a, const void* b);


int main(int argc, char* argv[]) {
    int num_elements = atoi(argv[1]);
    int* array = (int*)malloc(num_elements * sizeof(int));
    for(int i = 0; i < num_elements; i++) {
        array[i] = rand() % num_elements;
    }

    clock_t timer_start = clock();
    qsort(array, num_elements, sizeof(int), ComparisonFunc);
    clock_t timer_end = clock();

     printf("Time Elapsed (Sec): %f\n", 
            (double)(timer_end - timer_start) / CLOCKS_PER_SEC);

    free(array);
}

int ComparisonFunc(const void* a, const void* b) {
    return ( *(int*)a - *(int*)b );
}

