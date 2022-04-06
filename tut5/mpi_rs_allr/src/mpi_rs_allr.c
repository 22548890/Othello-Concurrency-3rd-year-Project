/* File:     mpi_allreduce.c
 *
 * Purpose:  Implement an algorithm that uses allreduce to 
 *           compute the sum of all the rank values. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "logger.h"

/*-------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
   int rank, ranksum = 0;

	/* TODO: Start up MPI */
	/* TODO: Get my rank among all the processes */
	/* TODO: Execute a broadcast reduction operation to compute the rank sum */
	/* TODO: If you're process 0, log the ranksum and print it 
		- uncomment the following two lines, but do not change them */ 
        //log_result(ranksum);
        //printf("Process %d: the rank sum is: %d \n ", rank, ranksum);

	/* TODO: Shut down MPI */

	return 0;
}
