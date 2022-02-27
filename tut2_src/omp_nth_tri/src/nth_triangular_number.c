#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include "logger.h"

double sum_critical(int n, int nt);
double sum_manual_private(int n, int nt);
double sum_reduction(int n, int nt);

#define DEBUG FALSE

int main(int argc, char *argv[])
{
    double sum = 0;
    int n, nt = 1, choice;

    if (argc != 4){
        printf("Incorrect Invocation, use: parallel_for <N> <number of threads> <choice: 0 critical, 1 manual reduction, 2 reduction> \n");
        return 0;
    } else {
        n = strtol(argv[1], NULL, 10);  
        nt = strtol(argv[2], NULL, 10);  
        choice = strtol(argv[3], NULL, 10);  
    }

    if (n < 0){
        printf("N cannot be negative");
        return 0;
    }

    switch (choice) {
        case 0: 
            sum = sum_critical(n, nt);  break;
        case 1: 
            sum = sum_manual_private(n, nt); break;
        default:  
            sum = sum_reduction(n, nt);
    }
    printf("Sum = %lf", sum);
    logger(64, sum);   
}

double sum_critical(int n, int nt) {
    double sum = 0;
    int i;
    //Todo: Calculate the nth triangular number using a parallel for
    //loop and the critical directrive to synchronise access to sum. 

    #pragma omp parallel for num_threads(nt)
        for (i = 1; i < n +1; i++) {
            #pragma omp critical
            sum+=i;
        }
    return sum;
}

double sum_manual_private(int n, int nt) {
    double sum = 0;
    //int num = n;
    int i;

    //Todo: Calculate the nth triangular number using a parallel for
    //loop more efficiently by using a local variable and the critical
    //directive to synchronise access to sum. 

     #pragma omp parallel for num_threads(nt) \
            private(i)
        for (i = 1; i < n +1; i++) {
            #pragma omp critical
            sum+=i;
        }
    return sum;
}

double sum_reduction(int n, int nt) {
    double sum = 0;
    int i;

    //Todo: Calculate the nth triangular number using a parallel for
    //loop and the reduction clause. 

     #pragma omp parallel for num_threads(nt) \
            reduction(+:sum) private(i)
        for (i = 1; i < n +1; i++) {
            #pragma omp critical
            sum+=i;
        }
    return sum;
}
