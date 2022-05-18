#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#define TRUE (1==1)
#define FALSE (1==0)

const int RMAX = 100000;
int thread_count; 
void print_list(int a[], int n, char* title);
void generate_list(int a[], int n);
void usage();
void serial_quicksort(int list_of_numbers[], int left, int right);
int is_sorted(int* array, int size);

/* Merge sorted arrays array_a and array_b */
void merge(int* array_a, int size_a, int* array_b, int size_b)
{
	int i = 0, j = 0; 
	int combined_size = size_a + size_b;
	int* merged_array = malloc(sizeof(int)*combined_size);

	for (int k = 0; k < combined_size; k++)
	{
		/* array_a[i] <= array_b[j] and not all items of array_a have been copied: copy next item from array_a  */
		if (array_a[i] <= array_b[j] && i < size_a)
		{
			merged_array[k] = array_a[i];
			i++;
		}
		/* array_a[i] >= array_b[j] and not all items of array_b have been copied: copy next item from array_b */
		else if (array_a[i] > array_b[j] && j < size_b)
		{
			merged_array[k] = array_b[j];
			j++;
		}
		/* all items of array_a have been copied => copy the rest of array_b */
		else if (i >= size_a)
		{
			merged_array[k] = array_b[j];
			j++;
		}
		/* all items of array_b have been copied => copy the rest of array_a */
		else if (j >= size_b)
		{
			merged_array[k] = array_a[i];
			i++;
		}
	}

	/* copy the merged array into array_a */	
	for (int k = 0; k < combined_size; k++)
		array_a[k] = merged_array[k];
	free(merged_array);
}

void merge_arrays(int* array, int size, int* indices)
{	
	int i = 0;
	int N = thread_count;
	while (N > 1)
	{
		for (i = 0; i < N; i++)
			indices[i] = (i*size)/N;
		indices[N] = size;
#		pragma omp parallel for
		for (i = 0; i < N; i+=2)
			merge(array+indices[i],indices[i+1]-indices[i],array+indices[i+1],indices[i+2]-indices[i+1]);
			
		N/=2;
	}
}

void serialQuickSort(int list_of_numbers[], int array_size)
{
  serial_quicksort(list_of_numbers, 0, array_size - 1);
}

int main(int argc, char **argv)
{
	int n;
	int i;
	double start_time, end_time;
	
	/* read parameters from command line */
	if (argc != 3) usage();
	thread_count = strtol(argv[1], NULL, 10);
	n = strtol(argv[2], NULL, 10);
	omp_set_num_threads(thread_count);
	
	printf ("Sorting an array of %d random elements using %d threads.\n", n, thread_count);
	
	/* Generate the unsorted array */
	int *array = malloc(sizeof(int)*n);
	generate_list(array, n);
#	ifdef DEBUG
	print_list(array, n, "Unsorted");
#	endif	
	
	/* Divide the array in chunks among the threads and store the start_timeing indices */
	int *indices = malloc(sizeof(int)*(thread_count+1));
	for (i = 0; i < thread_count; i++)
		indices[i] = (i*n)/thread_count;
	indices[thread_count] = n;
	

	/* The threads sort (using a serial quicksort) their chunks in parallel 
	 * and then the chunks are merged */ 
	start_time = omp_get_wtime();
#	pragma omp parallel for
	for (i = 0; i < thread_count; i++)
		serialQuickSort(array+indices[i], indices[i+1]-indices[i]);
	
	merge_arrays(array, n, indices);
	end_time = omp_get_wtime();
	
#	ifdef DEBUG
	print_list(array, n, "Sorted");
#	endif	
	printf ("The array is sorted: %s\n", is_sorted(array, n)?"TRUE":"FALSE");
	printf ("***************************************************************************\n");
	printf ("* Took %f s to quicksort an array of %d elements using %d threads. *\n", (end_time-start_time), n, thread_count);
	printf ("***************************************************************************\n");
	
	FILE *fp;
	fp = fopen("results.dat", "a");
	if (fp == NULL) {
		printf("I couldn't open results.dat for writing.\n");
		exit(0);
	}
	else {
		fprintf (fp, "%d %d %f\n", thread_count, n, (end_time - start_time));
	}

	return 0;
}

/* standard implementation of a quicksort done serially */
void serial_quicksort(int list_of_numbers[], int left, int right)
{
  int pivot, tmp_left, tmp_right;
 
  tmp_left = left;
  tmp_right = right;
  pivot = list_of_numbers[left];
  while (left < right)
  {
    while ((list_of_numbers[right] >= pivot) && (left < right))
      right--;
    if (left != right)
    {
      list_of_numbers[left] = list_of_numbers[right];
      left++;
    }
    while ((list_of_numbers[left] <= pivot) && (left < right))
      left++;
    if (left != right)
    {
      list_of_numbers[right] = list_of_numbers[left];
      right--;
    }
  }
  list_of_numbers[left] = pivot;
  pivot = left;
  left = tmp_left;
  right = tmp_right;

  if (left < pivot)
	serial_quicksort(list_of_numbers, left, pivot-1);
  if (right > pivot)
	serial_quicksort(list_of_numbers, pivot+1, right);

}

void print_list(int a[], int n, char* title) {
   int i;

   printf("%s:\n", title);
   for (i = 0; i < n; i++)
      printf("%d ", a[i]);
   printf("\n\n");
}  /* print_list */

void generate_list(int a[], int n) {
   int i;

   srandom(1);
   for (i = 0; i < n; i++)
      a[i] = random() % RMAX;
}  /* generate_list */

int is_sorted(int* array, int size)
{
	int i = 0;
	for (i = 1; i < size; i++)
	{
		if (array[i] < array[i-1])
			return FALSE;
	}
	return TRUE;
}

void usage()
{
	printf ("Usage: ./quicksort <thread_count> <n>, where\n<thread_count> is the number of threads to use, and\n<n> is the number of random elements to to sort.\n");
	exit(0);
}
