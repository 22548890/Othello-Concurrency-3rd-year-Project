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
#include "logger.h"
#include <omp.h>

/* Open the file for the current thread: in append mode */
FILE* open_logfile() {
    FILE* fptr = NULL;
    char* filename = NULL;

    if (!filename) {
        filename = malloc(64*sizeof(char));
        if (filename)
            sprintf(filename,"p%d.log", omp_get_thread_num());
        else
            return NULL;
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
    //free(fptr);
}

/* Successful deposit message printed and logged */
void log_dp(double amount, int acc_to, double balance) {
    FILE* fptr = open_logfile();
    fprintf(fptr,"deposit > acc: %d > amount: %9.2lf\n", acc_to, amount);
    fflush(fptr);
    close_logfile(fptr);

    printf("User %2d: Deposit %9.2f into acc %2d %21s %9.2lf\n", \
        omp_get_thread_num(), amount, acc_to, "-- Balance", balance);
}

/* Error msg printed: Deposit acc does not exist */
void log_dp_no_acc(double amount, int acc_to) {
        printf("\033[22;31mUser %2d: Deposit %9.2lf into acc %2d %64s \033[0m\n", \
            omp_get_thread_num(), amount, acc_to, "-- Error: account does not exist");
}

/* Successful withdrawal message printed and logged */ 
void log_wd(double amount, int acc_from, double balance) {
    FILE* fptr = open_logfile();
    fprintf(fptr,"withdrawal> acc: %d > amount: %9.2f\n", acc_from, amount);
    fflush(fptr);
    close_logfile(fptr);

    printf("User %2d, Withdraw %8.2f from acc %2d %21s %9.2f\n", omp_get_thread_num(), \
        amount, acc_from, "-- Balance", balance);
} 

/* Error msg printed: Withdrawal amount not available */ 
void log_wd_no_funds(double amount, int acc_from, double balance) {
    printf("\033[22;32mUser %2d, Withdraw %8.2f from acc %2d %21s %9.2f %s \033[0m\n", \
        omp_get_thread_num(), amount, acc_from, "-- Balance", \
        balance, "--insufficient funds");
}

/* Error msg printed: Withrawal acc does not exist */ 
void log_wd_no_acc(double amount, int acc_from) {
    printf("\033[22;31mUser %2d, Withdraw %8.2f from acc %2d %64s \033[0m\n", \
        omp_get_thread_num(), amount, acc_from, "-- Error: account does not exist");
}
    
/* Successful transfer message printed and logged */
void log_tr(double amount, int acc_from, int acc_to, double balance) {
    FILE* fptr = open_logfile();
    fprintf(fptr, "transfer > from acc: %d > to acc: %d > amount: %3.2lf\n", acc_from, acc_to, amount);
    fflush(fptr);
    close_logfile(fptr);

    printf("User %2d, Transfer %8.2f from acc %2d to acc %2d  -- Balance %9.2f\n", \
        omp_get_thread_num(), amount, acc_from, acc_to, balance);
}

/* Error msg printed: Transfer amount not available */
void log_tr_no_funds(double amount, int acc_from, int acc_to, double balance) {
    printf("\033[22;32mUser %2d, Transfer %8.2f from acc %2d %21s %8.2f -- insufficient funds\033[0m\n", \
        omp_get_thread_num(), amount, acc_from, "-- Balance", balance);
}

/* Error msg printed: transfer from acccount does not exist */
void log_tr_no_acc(double amount, int acc_from, int acc_to) {
    printf("\033[22;31mUser %2d, Transfer %8.2f from acc %2d to acc %2d %59s", \
        omp_get_thread_num(), amount, acc_from, acc_to, \
        "-- Error: account does not exist\033[0m\n");
}

/* Successful balance request message printed and logged */
void log_bl(int acc_from, double balance) { 
    FILE* fptr = open_logfile();
    fprintf(fptr, "balance > acc: %d\n\n", acc_from);   
    fflush(fptr);
    close_logfile(fptr);

    printf("User %2d, Balance %18s %2d %21s %9.2f\n", \
        omp_get_thread_num(), "acc", acc_from, "-- Balance", balance);
}

/* Error msg printed: Balance acc not available */
void log_bl_no_acc(int acc_from) { 
    printf("\033[22;31mUser %2d, Balance of acc %2d %76s \033[0m\n", \
        omp_get_thread_num(), acc_from, "-- Error: account does not exist");
}
 
/* Logging #pragma omp parallel */
void log_parallel() {
    FILE* fptr = open_logfile();
    fprintf(fptr,"omp parallel: %d threads\n", omp_get_num_threads());
    fflush(fptr);
    close_logfile(fptr);
}

/* Logging #pragma omp barrier */
void log_barrier() {
    FILE* fptr = open_logfile();
    fprintf(fptr,"omp barrier\n");
    fflush(fptr);
    close_logfile(fptr);
}

/* Logging #pragma omp critical */
void log_critical() {
    FILE* fptr = open_logfile();
    fprintf(fptr,"omp critical\n");
    fflush(fptr);
    close_logfile(fptr);
}

/* Logging omp_set_lock() */
void log_lock(int acc_num) {
    FILE* fptr = open_logfile();
    fprintf(fptr,"omp set lock %d\n", acc_num);
    fflush(fptr);
    close_logfile(fptr);
}

/* Logging omp_unset_lock() */
void log_unlock(int acc_num) {
    FILE* fptr = open_logfile();
    fprintf(fptr,"omp unset lock %d\n", acc_num);
    fflush(fptr);
    close_logfile(fptr);
}
