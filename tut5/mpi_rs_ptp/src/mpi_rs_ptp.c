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
   int comm_sz=0;
   int rank_sum=0;

   /* TODO: Start up MPI */
	MPI_Init(NULL, NULL);
	/* TODO: Get the number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	/* TODO: Get my rank among all the processes */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

   if (my_rank != 0) { 
      /* TODO: Send my_rank to process 0 */
      MPI_Send(&my_rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
      /* TODO: Execute a broadcast to receive the rank sum */
      //MPI_Recv(&rank_sum, sizeof(rank_sum)+1, MPI_DOUBLE, MPI_ALL_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE )
      MPI_Bcast(&rank_sum, 1, MPI_INT, 0, MPI_COMM_WORLD);
   } else {  
	/* TODO: Initialise ranksum with the rank of process 0 */
      rank_sum=0;
      int itemp=0;
	/* TODO: Receive the rank of each of the other processes and add it to ranksum */
   for (int i=1;i<comm_sz;i++){
      MPI_Recv(&itemp, 1, MPI_INT,i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      rank_sum+=itemp;
   }
	/* TODO: Execute a broadcast to send ranksum to all the processes */ 
      MPI_Bcast(&rank_sum, 1, MPI_INT, 0, MPI_COMM_WORLD);
	/* TODO: Uncomment the following to lines to log the result and print it
	 	- do not change the lines */
	log_result(rank_sum);
	printf("The sum of all the process's ranks are: %i\n", rank_sum);
   }

   /* Shut down MPI */
   MPI_Finalize();
   return 0;
}
