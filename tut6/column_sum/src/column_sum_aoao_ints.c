/* :se ts=4
 * File:
 * x.c
 * 
 * Purpose:
 * Computes the sum of the values in each row of an m by n matrix A and stores
 * it in an array x. 
 * 
 * Input:
 * m: number of rows, n: number of columns, version: 0-original, 1-efficient
 *
 * Output:
 * With DEBUG flag: vector x
 * Elapsed time 
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int efficiently = 0;

/* Functions */
void get_args(int argc, char* argv[], int* m_p, int* n_p);
void usage();
void gen_matrix(int** A, int m, int n);

/**
 * Function: get_args 
 * Purpose: Assign command line args 
 * In: argc, argv 
 * Out: m_p, n_p
 */
void get_args(int argc, char* argv[], int* m_p, int* n_p) {
	if (argc != 4) usage(argv[0]);
	*m_p = strtol(argv[1], NULL, 10);
	*n_p = strtol(argv[2], NULL, 10);
	efficiently = strtol(argv[3], NULL, 10);	
	if (*m_p <= 0 || *n_p <= 0) usage(argv[0]);
} /* get_args */

/**
 * Function: usage 
 * Purpose: print a message showing what command line arguments are needed 
 * In: prog_name 
 * Out: 
 */
void usage(char *prog_name) {
	fprintf(stderr, "Usage: %s <m> <n> <version> (version 0 - original or 1 - efficient)>\n", prog_name);
	exit(0);
} /* usage */

/**
 * Function: gen_matrix 
 * Purpose: Assign values to the matrix  
 * In: A, m, n 
 * Out: A
 */
void gen_matrix(int** A, int m, int n) {
	int i, j;
	for(i = 0; i < m; i++) {
		for(j = 0; j < n; j++) {
			A[i][j] = i; 
		}
	}
}

/**
 * Function: clear 
 * Purpose: Free memory 
 * In: A, x 
 * Out: A, x
 */
void clear(int** A, int* x, int m) {
	int i;
	for(i = 0; i < m; i++) free(A[i]);
	free(A);
	free(x);
}

/**
 * Function: add_col_vals
 * Purpose: Calculate the sum of the values of each column 
 * In: A, m, n
 * Out: x
 */
void add_col_vals(int** A, int m, int n, int* x) {
	int i, j;
	for(j = 0; j < n; j++) {
		x[j] = 0;
		for(i = 0; i < m; i++) {
			x[j] += A[i][j];
		}
	}
}

/**
 * Function: add_col_vals_efficiently
 * Purpose: 
 * In: A, m, n
 * Out: x
 */
void add_col_vals_efficiently(int** A, int m, int n, int* x) {
}

/**
 * Function: main 
 * Purpose: Define data structures, call and time the chosen function 
 * In: argc, argv 
 * Out: 
 */
int main(int argc, char* argv[]) {
	clock_t start, end;
	int i, m, n; 
	int** A = NULL;	
	int* x = NULL;	
	
	get_args(argc, argv, &m, &n);

	x = calloc(n,sizeof(int)); /* malloc(n,sizeof(int)) + memset(A,0,n*sizeof(int)) */
	A = (int**) malloc(m*sizeof(int *));
	for (i = 0; i < m; i++) {
		A[i] = malloc(n*sizeof(int));
		#ifdef DEBUG 
			fprintf(stderr,"Allocate memory for row A[%d] at address %p", i, &A[i]); 
			fprintf(stderr,"  (x[%d]= %d)\n", i, x[i]); 
		#endif
	}

	if ((A != NULL) && (x != NULL)) {
		printf("Setting up the Matrix...\n");
		gen_matrix(A, m, n);
		printf("Adding the solumn values ...\n");
		if (efficiently) {	
			start = clock();
			add_col_vals_efficiently(A, m, n, x);	
			end = clock();
		} else {
			start = clock();
			add_col_vals(A, m, n, x);	
			end = clock();
		}
			printf ("Adding column values: took %.3lf seconds.\n", difftime(end, start) / CLOCKS_PER_SEC);
	} else {
		fprintf(stderr,"Could not allocate memory for the matrix or the vector\n");
	}

	clear(A, x, m);
	return 0;
}
