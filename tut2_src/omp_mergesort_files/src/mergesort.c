#include <stdio.h> 
#include <stdlib.h>
#include <omp.h>
#include "fileio.h"

#define DEBUG 1

int* merge_sorted_arrays(int* arr1, int arr1_sz, int* arr2, int arr2_sz);
void print_array(int a[], int n, char* title);
int is_sorted(int* array, int size);
int cmpfunc (const void * a, const void * b);
void usage();

int main(int argc, char* argv[]) {
    int nt, num_ranges, range_size; 
    int* ranges; 
 
    if (argc != 4) usage();

    /* Read command line parameters */
    nt = strtol(argv[1], NULL, 10);
    num_ranges = strtol(argv[2], NULL, 10);
    range_size = strtol(argv[3], NULL, 10);

    ranges = malloc(sizeof(int)*(num_ranges + 1));   
    for (int i = 0; i <= num_ranges; i++) { 
        ranges[i] = range_size * i; 
    }

    int* arr1;
    arr1 = malloc(range_size*sizeof(int));
    char* filename1 = malloc(sizeof(char)*32);

    int i = 0; 

    int start1 = ranges[i], end1 = ranges[i + 1] - 1; 
    int size1 = ranges[i + 1] - ranges[i]; 

    /* Read the unsorted array from the given input file */
    sprintf(filename1, "data/unsorted_%d_%d.txt", start1, end1);
    if (read_array_from_file(filename1, arr1, size1) != 0) {
        printf("Error reading array from %s\n", filename1);
    }

    /* Sort arr1 using the built-in quick sort algorithm */
    qsort(arr1, size1, sizeof(int), cmpfunc);
    

    /* Write the sorted array to a log file */
    sprintf(filename1, "sorted_%d_%d.log", start1, end1); 
    if (write_array_to_file(filename1, arr1, size1) != 0) {
        printf("Error writing arr1 to %s\n", filename1);
    }

    /* Check whether the array is sorted */
#   ifdef DEBUG
    if (is_sorted(arr1, size1) == 0) {
        printf("Error: the array is not sorted.\n");
        if (size1 < 50) print_array(arr1, range_size, "list of numbers: ");
    } 
#   endif

    return 0;
}

/* 
 * Merge two sorted arrays into tmp in sorted order and return 
 */
int* merge_sorted_arrays(int* arr1, int arr1_sz, int* arr2, int arr2_sz)
{
    int i = 0, j = 0, k = 0;
    int* tmp = NULL;
    int tmp_size;

    /* Todo: Merge arr1 and arr2 into tmp in sorted order */
    

    return tmp;    
}

/*
 *  Print array 
 */ 
void print_array(int a[], int n, char* title) {
    printf("%s:\n", title);
    for (int i = 0; i < n; i++)
        printf("%d ", a[i]);
    printf("\n");
}  

/* 
 * Returns 1 if array is sorted, 0 if not 
 */ 
int is_sorted(int* array, int size)
{
    int i = 0;
    
    if (size <= 1) return 1;
    while (i < size-1 && array[i] <= array[i+1]) i++; 

    if (i < size-1) return 0;
    else return 1; 
}

/* 
 * Compare to values: required by quicksort 
 */
int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

/* 
 * Lists the required command-line parameters 
 */ 
void usage()
{
    printf ("Usage: ./mergesort <nt> <num_ranges> <range_size>, where \
    \n<nt> is how many threads to use, \
    \n<num_ranges> is the number of files to sort and \
    \n<range_size> is the number of numbers per file.\n\n");

    exit(0);
}
