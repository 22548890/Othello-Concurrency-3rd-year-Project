#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
void Hello(void); /* Thread function */
int main(int argc, char* argv[]) {
/* Get number of threads from command line */
int thread_count = atoi(argv[1]);
# pragma omp parallel num_threads(thread_count)
Hello();
return 0;
} /* main */
void Hello(void){
int my_rank = omp_get_thread_num();
int thread_count = omp_get_num_threads();
FILE *fp;
char filepath[256];
snprintf (filepath, sizeof(filepath), "thr%d.log", my_rank);
fp = fopen(filepath, "w+");
fprintf(fp, "Hello from thread %d of %d\n", my_rank, thread_count);//fprintf(file, "this is a test %d\n", integer)
} /* Hello */