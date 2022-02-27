/**
  * Depending on the usage, the logger may
  * cause the system to run out of memory while
  * trying to print multiple large matricies or vectors.
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <omp.h>

#define HAS_VECTOR    (EID & 1)
#define HAS_MATRIX    (EID & 2)
#define HAS_PROC_ID   (EID & 4)
#define HAS_THREAD_ID (EID & 8)
#define HAS_FILENAME  (EID & 16)
#define HAS_MESSAGE   (EID & 32)
#define HAS_DOUBLE    (EID & 64)

void print_int_vector(int* A, int size, FILE* fptr);
void print_int_mat(int** M, int x, int y, FILE* fptr);
void print_double(double dvalue, FILE* fptr);
void print_current_omp_rank();

/** EID is a binary number representing what args are present
  * 00000000 - not used
  * 00000001 - contains a vector (int) followed by it's length as an int
  * 00000010 - contains a matrix (int) followed by it's length and width as ints
  * 00000100 - contains a processorID
  * 00001000 - contains a ThreadID
  * 00010000 - contains a filename
  * 00100000 - contains a Message
  */
void logger(int EID, ... ) {
  va_list valist;
  int* array = NULL;
  int** Matrix = NULL;
  double dvalue = 0;
  int size = 0;
  int m = 0, n = 0;
  int ThreadProcID = 0;
  char* filename = NULL;
  char* Message = NULL;
  FILE* fptr = NULL;

  /* start collecting the included arguments */
  va_start(valist, EID);

  if (HAS_DOUBLE) {
    dvalue = va_arg(valist, double);
  }
  if (HAS_VECTOR) {
    array = va_arg(valist, int*);
    size = va_arg(valist, int);
  }
  if (HAS_MATRIX) {
    Matrix = va_arg(valist, int**);
    m = va_arg(valist, int);
    n = va_arg(valist, int);
  }
  if (HAS_PROC_ID || HAS_THREAD_ID)
    ThreadProcID = va_arg(valist, int);
  if (HAS_FILENAME)
    filename = va_arg(valist, char*);
  if (HAS_MESSAGE)
    Message = va_arg(valist, char*);

  /* clean memory reserved for valist */
  va_end(valist);

  /* open the file and print to it the content required */
  if (!filename) {
    filename = malloc(64*sizeof(char));
    if (filename)
      sprintf(filename,"ex_%d_p%d.log", EID, ThreadProcID);
    else
      return;
  }
  fptr = fopen(filename, "a");
  if (!fptr)
    return;
  if (HAS_DOUBLE)
    print_double(dvalue, fptr);
  if (HAS_VECTOR)
    print_int_vector(array, size, fptr);
  if (HAS_MATRIX)
    print_int_mat(Matrix, m, n, fptr);
  if (HAS_MESSAGE) {
    fprintf(fptr, "> %s", Message);
    fflush(fptr);
  }
  fclose(fptr);
  // free(fptr);
  free(filename);
}

/* This function prints out the content of a vector to the log file */
void print_int_vector(int* A, int size, FILE* fptr) {
  int i;
  fprintf(fptr,"> Vector content:\n");
  fflush(fptr);
  for(i = 0; i < size; i++) {
    fprintf(fptr,"Vector[%d] = %d\n", i, A[i]);
    fflush(fptr);
  }
}

/* This function prints out the content of a matrix to the log file */
void print_int_mat(int** M, int x, int y, FILE* fptr) {
  int i, j;
  fprintf(fptr,"> Matrix content:\n");
  fflush(fptr);
  for(i = 0; i < x; i++) {
    for(j = 0; j < y; j++) {
      fprintf(fptr,"Matrix[%d][%d] = %d\n", i, j, M[i][j]);
      fflush(fptr);
    }
  }
}

/* This function prints out a double result */
void print_double(double dvalue, FILE* fptr) {
  fprintf(fptr,"%lf\n", dvalue); 
  fflush(fptr);
}

/* This function prints out the current OpenMP thread rank */ 
void print_current_omp_rank(FILE* fptr) {
  fprintf(fptr,"%d", omp_get_thread_num());
  fflush(fptr);
}
