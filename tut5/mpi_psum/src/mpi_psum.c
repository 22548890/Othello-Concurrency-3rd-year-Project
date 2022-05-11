/* vim settings: :se ts=4 
 * File: mpi_psum.c
 *
 * Purpose: Implement a parallel algorithm for calculating the sum 
 *          of a list of values given inside an input file 
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "logger.h"

#define MAX 100
#define DEBUG 1 

void usage(char* prog_name);
int process_zero(char *input_file);
void process_i();
void read_vector(int *vector, int *n_p, char* input_file);
void print_vector(int *x, int n, char *msg);

/*-------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
	int my_rank;
	char *input_file = NULL;
	
	if (argc != 2) {
		usage(argv[0]);
	} else {
	 	input_file = malloc(sizeof(char)*strlen(argv[1]));	
		strcpy(input_file, argv[1]);
	}

	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	if (my_rank == 0) {
		int max = process_zero(input_file);
		printf("The max value in %s is %d \n", input_file, max);
		log_result(max);
	} else {
		process_i();
	}

	MPI_Finalize();

	return 0;
}  /* main */

/*---------------------------------------------------------------------------
 * Function: usage 
 * Purpose:  print a message showing what command line arguments are needed 
 */
void usage(char* prog_name){
	fprintf(stderr, "Usage: %s <filename> \n", prog_name);
	exit(0);
} /* usage */


/*-------------------------------------------------------------------
 * Function: process_zero 
 * Purpose: 
 * - Read the values from the input file
 * - Scatter the values equally among the processes  
 * - Find the maximum value in the locally received subvector
 * - Find the global max of the local maximum values  
 */

int process_zero(char *input_file) {
	int *vector_full = NULL;
	int *vector_local = NULL;
	int size_full = 0, size_local = 0;
	int global_max = 0;

	int np = 0;
	int comm_sz = 0;

	/* TODO: get comm_sz: e.g., 2 */ 
 	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

	/* TODO: allocate memory */
    vector_full = malloc(sizeof(int)*strlen(input_file));

	/* TODO: read values from input file and store in vector_full: e.g., 1 2 3 4 5 6 7 8 9 */
	read_vector(vector_full, &np , input_file);

	size_full = np;	

	#ifdef DEBUG 
	print_vector(vector_full, size_full, "Full vector on proc");
	#endif

	/* TODO: calculate size_local: size of subvectors, e.g., 9 / 2 = 4 */
	/* 	- ignore last numbers if they can't be equally divided */
	/* TODO: broadcast size of subvectors */
	size_local = (int) size_full/comm_sz;
	MPI_Bcast(&size_local, 1, MPI_INT, 0, MPI_COMM_WORLD);

	/* TODO: allocate memory */
	vector_local = malloc(size_local * sizeof(int));

	/* TODO: scatter vector equally among all processes using scatter, e.g., Proc 0 receives 1 2 3 4 */ 
	MPI_Scatter(vector_full, size_local , MPI_INT ,vector_local, size_local, MPI_INT, 0, MPI_COMM_WORLD);

	#ifdef DEBUG 
	print_vector(vector_local, size_local, "Subvector on proc");
	#endif

	/* TODO: find the max value in subvector, e.g., 4 */ 
	int max = 0;
	for (int i = 1; i < size_local; ++i) {
    if ( *(vector_local) <  *(vector_local+i)){
      *(vector_local) = *(vector_local+i);
    } 	
	max = *(vector_local);
  }
	/* TODO: find the global max value using the collective Reduce command, e.g., 8 */
	MPI_Reduce(&max, &global_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
 
	/* TODO: free memory */
	free(vector_full);
    free(vector_local);
	return global_max;
}

/*-------------------------------------------------------------------
 * Function: process_i 
 * - Receive a subvector of values via the scatter command
 * - Find the maximum value in the locally received subvector
 * - Send the local maximum to process 0 in a collective reduce communication command
 */

void process_i() {
	int *vector_local = NULL;
	//int *vector_full = NULL;
	//int size_full = 1;
	int size_local = 0;
	
	/* TODO: receive size of subvector from pro 0 using a broadcast */
	MPI_Bcast(&size_local, 1, MPI_INT, 0, MPI_COMM_WORLD);
	/* TODO: allocate memory for subvector and receive subvector from proc 0 using scatter */ 
	vector_local = (int*) malloc(size_local * sizeof(int));
	MPI_Scatter(NULL, 1 , MPI_INT ,vector_local, size_local, MPI_INT, 0, MPI_COMM_WORLD);

	#ifdef DEBUG 
	print_vector(vector_local, size_local, "Subvector on proc");
	#endif

	/* TODO: find the maximum value in the locally received subvector */
		int local_max = 0;
		for (int i = 1; i < size_local; ++i) {
		if ( *(vector_local) <  *(vector_local+i)){
		*(vector_local) = *(vector_local+i);
		} 	
		local_max = *(vector_local);
	}
	int global_max;
	/* TODO: find the global maximum value using the collective Reduce command */
	MPI_Reduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
 
	/* TODO: free memory */
 	free(vector_local); 
  }

/*-------------------------------------------------------------------
 * Function: read_vector
 * Purpose: Read a vector of integer values from a file 
 */

void read_vector(int *vector, int *n_p, char* input_file) {
	FILE* fptr = NULL;
	int num;

	fptr = fopen(input_file, "r");
	if (fptr != NULL) {
		int i = 0;
		while(fscanf(fptr, "%d", &num) > 0) {
			vector[i] = num;
			i++;
		}
		*n_p = i;
		fclose(fptr);
	} else {
		fprintf(stderr, "Can't open data file");	
	}
}
/*-------------------------------------------------------------------
 * Function: print_vector
 * Purpose: Print a msg and a vector of integer values
 */
void print_vector(int *x, int n, char *msg) {

	int my_rank; 
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	printf("%s %d: ", msg, my_rank);
	for (int i = 0; i < n; i++) {
		printf("%d ", x[i]);
	}
	printf("\n");
}
