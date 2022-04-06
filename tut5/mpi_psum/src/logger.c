/**
  * !!! DO NOT CHANGE ANYTHING IN THIS FILE !!!
  * THIS FILE IS OVERWRITTEN DURING TESTING 
  * SO THAT YOUR CODE IS COMPILED WITH THE ORIGINAL VERSION
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "logger.h"
#include <mpi.h>

/* Open the file for the current thread: in append mode */
FILE* open_logfile() {
    FILE* fptr = NULL;
    char* filename = NULL;
    int rank;

    if (!filename) {
        filename = malloc(64*sizeof(char));
        if (filename) {
            MPI_Comm_rank(MPI_COMM_WORLD, &rank);
            sprintf(filename,"mpi_psum_p%d.log", rank);
        } else {
            return NULL;
        }
    }

    fptr = fopen(filename, "a");
    if (!fptr)
        return NULL; 

    free(filename);
    return fptr;
}

/* Close the logfile pointed to by fptr */
void close_logfile(FILE* fptr) {
    fclose(fptr);
}

/* Logging that a message was sent */
void log_send() { 
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    FILE *fptr = open_logfile();
    fprintf(fptr,"Proc %d: send a message\n", rank);
    close_logfile(fptr);
}

/* Logging that a message was received */
void log_recv() { 
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    FILE *fptr = open_logfile();
    fprintf(fptr,"Proc %d: received a message\n", rank);
    close_logfile(fptr);
}

/* Logging a broadcast message */
void log_bcast() { 
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    FILE *fptr = open_logfile();
    fprintf(fptr,"Proc %d: called Bcast\n", rank);
    close_logfile(fptr);
}


/* Logging that a reduce operation was executed */
void log_reduce() {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    FILE *fptr = open_logfile();
    fprintf(fptr,"Proc %d: called Reduce\n", rank);
    close_logfile(fptr);
}

/* Logging that an allreduce operation was executed */
void log_allreduce() {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    FILE *fptr = open_logfile();
    fprintf(fptr,"Proc %d: called Allreduce\n", rank);
    close_logfile(fptr);
}

/* Logging that a Scatter operation was executed */
void log_scatter() {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    FILE *fptr = open_logfile();
    fprintf(fptr,"Proc %d: called Scatter \n", rank);
    close_logfile(fptr);
}

/* Logging that a Gather operation was executed */
void log_gather() {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    FILE *fptr = open_logfile();
    fprintf(fptr,"Proc %d: called Gather\n", rank);
    close_logfile(fptr);
}

/* Logging that an Allgather operation was executed */
void log_allgather() {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    FILE *fptr = open_logfile();
    fprintf(fptr,"Proc %d: called Allgather\n", rank);
    close_logfile(fptr);
}

/* Logging a vector */
void log_vector(int *vector, int n, char *msg) { 
    FILE *fptr = open_logfile();
    fprintf(fptr,"%s: ", msg);
    for (int i = 0; i < n; i++) {
        fprintf(fptr, "%d ", vector[i]);
    }
    fprintf(fptr, "\n");
    close_logfile(fptr);
}

/* Logging a result */
void log_result(int res) { 
    FILE *fptr = open_logfile();
    fprintf(fptr,"The final result is %d\n", res);
    close_logfile(fptr);
}

/* Logging a result */
void log_msg(char *msg) { 
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    FILE *fptr = open_logfile();
    fprintf(fptr,"Proc %d: %s\n", rank, msg);
    close_logfile(fptr);
}
