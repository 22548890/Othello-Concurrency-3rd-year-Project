/* vim:
 * :se tw=2
 *
 * File: 
 * mpi_hello.c
 *
 * Purpose: 
 * A "hello world" program that uses MPI
 */
#include <stdio.h>
#include <string.h> 
#include <mpi.h>   
#include "logger.h"

#define MAX 100

int main(void) {
	char greeting[MAX];  
	int my_rank = 0, comm_sz = 0;

	/* TODO: Start up MPI */
	/* TODO: Get the number of processes */
	/* TODO: Get my rank among all the processes */

	if (my_rank != 0) {
		/* Create greeting message for process my_rank */
		sprintf(greeting, "Greetings from process %d of %d!", my_rank, comm_sz);
		/* TODO: Uncomment this line once parallelised - do not change it */
		//log_msg(greeting); 

		/* TODO: Send message to process 0 */

	} else {
		sprintf(greeting, "Greetings from process %d of %d!", my_rank, comm_sz);
		/* TODO: Uncomment these lines once parallelised - do not change it */
		//log_msg(greeting); 
		//printf("%s\n", greeting);

		/* TODO: Receive a message from each of the other processes and print it*/
			/* TODO: Uncomment this line once parallelised - do not change it */
			//printf("%s\n", greeting);
	}

	/* TODO: Shut down MPI */

	return 0;
}
