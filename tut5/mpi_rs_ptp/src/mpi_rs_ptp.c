/* File:       
 *    mpi_rs_ptp.c
 *
 */
#include <stdio.h>
#include <string.h>
#include <mpi.h>

#include "logger.h"

int main(void) {
   int my_rank = 0; /* My process rank */

   /* TODO: Start up MPI */
   /* TODO: Get the number of processes */
   /* TODO: Get my rank among all the processes */

   if (my_rank != 0) { 
      /* TODO: Send my_rank to process 0 */
      /* TODO: Execute a broadcast to receive the rank sum */

   } else {  
	/* TODO: Initialise ranksum with the rank of process 0 */
	/* TODO: Receive the rank of each of the other processes and add it to ranksum */
	/* TODO: Execute a broadcast to send ranksum to all the processes */ 

	/* TODO: Uncomment the following to lines to log the result and print it
	 	- do not change the lines */
	//log_result(ranksum);
	//printf("The sum of all the process's ranks are: %i\n", ranksum);
   }

   /* Shut down MPI */

   return 0;
}
