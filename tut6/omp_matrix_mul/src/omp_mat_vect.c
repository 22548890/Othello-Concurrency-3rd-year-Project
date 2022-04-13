/* File:     
 *     omp_mat_vect.c 
 *
 *
 * Purpose:  
 *     Computes a parallel matrix-vector product.  Matrix
 *     is distributed by block rows.  Vectors are distributed by 
 *     blocks.  Unless the DEBUG flag is turned on this version 
 *     uses a random number generator to generate A and x.
 *
 * Input:
 *     None unless compiled with DEBUG flag.
 *     With DEBUG flag, A, x
 *
 * Output:
 *     y: the product vector
 *     Elapsed time for the computation
 *
 * Compile:  
 *    gcc -g -Wall -o omp_mat_vect omp_mat_vect.c -lpthread
 * usage:
 *    omp_mat_vect <thread_count> <m> <n>
 *
 * Notes:  
 *     1.  Storage for A, x, y is dynamically allocated.
 *     2.  Number of threads (thread_count) should evenly divide both 
 *         m and n.  The program doesn't check for this.
 *     3.  We use a 1-dimensional array for A and compute subscripts
 *         using the formula A[i][j] = A[i*n + j]
 *     4.  Distribution of A, x, and y is logical:  all three are 
 *         globally shared.
 *     5.  DEBUG compile flag will prompt for input of A, x, and
 *         print y
 *     6.  Uses the OpenMP library function omp_get_wtime() to
 *         return the time elapsed since some point in the past
 *
 * IPP:    Section 5.9 (pp. 253 and ff.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Serial functions */
void get_args(int argc, char *argv[], int *thread_count_p, 
      int *m_p, int* n_p);
void usage(char *prog_name);
void gen_matrix(double A[], int m, int n);
void read_matrix(char *prompt, double A[], int m, int n);
void gen_vector(double x[], int n);
void read_vector(char *prompt, double x[], int n);
void print_matrix(char *title, double A[], int m, int n);
void print_vector(char *title, double y[], double m);

/* Parallel function */
void omp_mat_vect(double A[], double x[], double y[],
      int m, int n, int thread_count);

/*------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
   int thread_count;
   int m, n;
   double *A;
   double *x;
   double *y;

   get_args(argc, argv, &thread_count, &m, &n);

   A = malloc(m*n*sizeof(double));
   x = malloc(n*sizeof(double));
   y = malloc(m*sizeof(double));
   
   #ifdef DEBUG
   read_matrix("Enter the matrix", A, m, n);
   print_matrix("We read", A, m, n);
   read_vector("Enter the vector", x, n);
   print_vector("We read", x, n);
   #else
   gen_matrix(A, m, n);
   gen_vector(x, n);
   #endif

   omp_mat_vect(A, x, y, m, n, thread_count);

   #ifdef DEBUG
   print_vector("The product is", y, m);
   #endif

   free(A);
   free(x);
   free(y);

   return 0;
}  /* main */


/*------------------------------------------------------------------
 * Function:  get_args
 * Purpose:   Get command line args
 * In args:   argc, argv
 * Out args:  thread_count_p, m_p, n_p
 */
void get_args(int argc, char *argv[], int *thread_count_p, 
      int *m_p, int* n_p)  {

   if (argc != 4) usage(argv[0]);
   *thread_count_p = strtol(argv[1], NULL, 10);
   *m_p = strtol(argv[2], NULL, 10);
   *n_p = strtol(argv[3], NULL, 10);
   if (*thread_count_p <= 0 || *m_p <= 0 || *n_p <= 0) usage(argv[0]);

}  /* get_args */

/*------------------------------------------------------------------
 * Function:  usage
 * Purpose:   print a message showing what the command line should
 *            be, and terminate
 * In arg :   prog_name
 */
void usage (char *prog_name) {
   fprintf(stderr, "usage: %s <thread_count> <m> <n>\n", prog_name);
   exit(0);
}  /* usage */

/*------------------------------------------------------------------
 * Function:    read_matrix
 * Purpose:     Read in the matrix
 * In args:     prompt, m, n
 * Out arg:     A
 */
void read_matrix(char *prompt, double A[], int m, int n) {
   int             i, j;

   printf("%s\n", prompt);
   for (i = 0; i < m; i++) 
      for (j = 0; j < n; j++)
         scanf("%lf", &A[i*n+j]);
}  /* read_matrix */

/*------------------------------------------------------------------
 * Function: gen_matrix
 * Purpose:  Use the random number generator random to generate
 *    the entries in A
 * In args:  m, n
 * Out arg:  A
 */
void gen_matrix(double A[], int m, int n) {
   int i, j;
   for (i = 0; i < m; i++)
      for (j = 0; j < n; j++)
         A[i*n+j] = random()/((double) RAND_MAX);
}  /* gen_matrix */

/*------------------------------------------------------------------
 * Function: gen_vector
 * Purpose:  Use the random number generator random to generate
 *    the entries in x
 * In arg:   n
 * Out arg:  A
 */
void gen_vector(double x[], int n) {
   int i;
   for (i = 0; i < n; i++)
      x[i] = random()/((double) RAND_MAX);
}  /* gen_vector */

/*------------------------------------------------------------------
 * Function:        read_vector
 * Purpose:         Read in the vector x
 * In arg:          prompt, n
 * Out arg:         x
 */
void read_vector(char *prompt, double x[], int n) {
   int   i;

   printf("%s\n", prompt);
   for (i = 0; i < n; i++) 
      scanf("%lf", &x[i]);
}  /* read_vector */


/*------------------------------------------------------------------
 * Function:  omp_mat_vect
 * Purpose:   Multiply an mxn matrix by an nx1 column vector
 * In args:   A, x, m, n, thread_count
 * Out arg:   y
 */
void omp_mat_vect(double A[], double x[], double y[],
      int m, int n, int thread_count) {
   int i, j;
   double start, finish, elapsed;

   start = omp_get_wtime();
#  pragma omp parallel for num_threads(thread_count)  \
      default(none) private(i, j)  shared(A, x, y, m, n)
   for (i = 0; i < m; i++) {
      y[i] = 0.0;
      for (j = 0; j < n; j++)
         y[i] += A[i*n+j]*x[j];
   }
   finish = omp_get_wtime();
   elapsed = finish - start;
   printf("Elapsed time of multiplication part only = %e seconds\n", elapsed);

}  /* omp_mat_vect */


/*------------------------------------------------------------------
 * Function:    print_matrix
 * Purpose:     Print the matrix
 * In args:     title, A, m, n
 */
void print_matrix(char *title, double A[], int m, int n) {
   int   i, j;

   printf("%s\n", title);
   for (i = 0; i < m; i++) {
      for (j = 0; j < n; j++)
         printf("%4.1f ", A[i*n + j]);
      printf("\n");
   }
}  /* print_matrix */


/*------------------------------------------------------------------
 * Function:    print_vector
 * Purpose:     Print a vector
 * In args:     title, y, m
 */
void print_vector(char *title, double y[], double m) {
   int   i;

   printf("%s\n", title);
   for (i = 0; i < m; i++)
      printf("%4.1f ", y[i]);
   printf("\n");
}  /* print_vector */
