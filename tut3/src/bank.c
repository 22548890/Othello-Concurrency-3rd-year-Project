/* 
 * File: bank.c 
 * 
 * An account, Account holders, Deposit and Withdrawel transactions 
 *         
 * Run: . run.sh 
 * 
 * TODO: Currently, only one transaction list is read in and execute by one thread 
 *       Parallelise this, so that 
 *       - multiple threads can each read a transaction list (one transaction list per thread)       
 *       - each thread can execute its own list of transactions in parallel with the other threads 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loader.h"
#include "parser.h"
#include "logger.h"
#include "omp.h"

void usage(char* prog_name);
int get_num_files_from_args(int argc, char* argv[]);
char* get_fname_from_args(int argc, char* argv[], int thread_num);
void deposit(int acc_num, double amount);
void withdrawal(int acc_num, double amount);
void transfer(int acc_from, int acc_to, double amount);
void acc_balance(int acc_num);

#define MAX_ACCS 10

double *account_balances;
int num_accounts = MAX_ACCS;

/*------------------------------------------------------------------
 * @brief  usage
 *         print a message showing what the command line should be, and terminate
 * In arg: prog_name
 */
void usage (char* prog_name) {
    fprintf(stderr, "usage: %s <num_transaction files> <transaction file> {<transaction file.txt>}\n", prog_name);
    exit(0);
}

/*------------------------------------------------------------------
 * @brief  get_num_files_from_args
 *         Get the 2nd command line arg
 * In args:  argc, argv
 * Out args: thread_count 
 */
int get_num_files_from_args(int argc, char* argv[]) {

    if (argc < 3) usage(argv[0]);

    int num_files = strtol(argv[1], NULL, 10);
    if (num_files <= 0) usage(argv[0]);

    return num_files;

}  /* get_num_files_from_args */

/*------------------------------------------------------------------
 * @brief  get_fname_from_args
 *         Get the name of the transaction file for thread thread_num 
 * In args:  argc, argv
 * Out args: thread_num 
 */
char* get_fname_from_args(int argc, char* argv[], int id) {

    if (argc < (3 + id)) usage(argv[0]);
#   ifdef DEBUG
    printf("Returning argument 2+%2d: transaction file for user %2d = %s\n", id, id, argv[2+id]);
#   endif

    return argv[2 + id];

}  /* get_num_files_from_args */

/*------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int num_files = get_num_files_from_args(argc, argv);

    /* Declare and initialise accounts */
    num_accounts = MAX_ACCS;
    account_balances = (double*) calloc(num_accounts, sizeof(double));  
    if (account_balances == NULL) { 
        fprintf(stderr,"Memory could not be allocated, exiting\n");
        exit(0);    
    }

    if (num_files == 1) {
        printf("\033[22;32m Warning: Only one thread reading one transaction file supported.\033[0m\n");
    }
     
    /* Parse file and load batch of transactions into transaction_list */ 
    struct transaction_t* transaction_list;
    transaction_list = parse_transaction_file(get_fname_from_args(argc,argv,0));
    if (transaction_list == NULL) { 
        printf("Something bad happened, parse_transaction_file(%s) returned no list\n", \
        get_fname_from_args(argc,argv,0));
        exit(0);
    }

    /* Execute batch of transactions */
    while (transaction_list != NULL) {
        if (transaction_list->type == DP_T){
            deposit(transaction_list->dest, transaction_list->amount);
        } else if (transaction_list->type == WD_T){
            withdrawal(transaction_list->src, transaction_list->amount);
        } else if (transaction_list->type == TR_T){
            transfer(transaction_list->src, transaction_list->dest, transaction_list->amount);
        } else if (transaction_list->type == BL_T){
            acc_balance(transaction_list->src);
        }
        transaction_list = transaction_list->next;
    }

    printf("    *********************************************************   \n");
    for (int i = 0; i < num_accounts; i++) {
        printf("Account %2d balance after completion of transaction batch: %9.2f\n", i, account_balances[i]);
    }
    printf("    *********************************************************   \n");
    return 0;    
}

/*--------------------------------------------------------------------
 * @brief deposit 
 *        Add amount to balance 
 * @param acc_num: Account number
 * @param amount:  Amount to deposit 
 * @param balance: Balance of acc_num  
 */
void deposit(int acc_num, double amount)
{
    /* Valid account */
    if (acc_num < num_accounts) { 
        log_dp(amount, acc_num, account_balances[acc_num]); 
        account_balances[acc_num] += amount;
    } else {
        log_dp_no_acc(amount, acc_num);
    }
} 

/*--------------------------------------------------------------------
 * @brief withdrawal 
 *    If amount available, subtract amount from balance 
 * @param acc_num: Account Number
 * @param amount:  Amount to withdraw 
 * @param balance: Balance of acc_num  
 */
void withdrawal(int acc_num, double amount)
{
    /* Valid account */
    if (acc_num < num_accounts) {

      /* Amount available in from account */
      if (amount <= account_balances[acc_num]) { 
          log_wd(amount, acc_num, account_balances[acc_num]); 
          account_balances[acc_num] -= amount;
      } else {
          log_wd_no_funds(amount, acc_num, account_balances[acc_num]); 
      }
    } else {
        log_wd_no_acc(amount, acc_num);
    }
}

/*--------------------------------------------------------------------
 * @brief transfer 
 *     If amount available in acc_from, 
 *     subtract amount from acc_from balance and add to acc_to balance
 * @param acc_from: Number of account from which money is transferred
 * @param acc_to:   Number of account to which money is transferred 
 * @param amount:   Amount to transfer 
 * @param balance:  Balance of acc_from  
 */
void transfer(int acc_from, int acc_to, double amount) {
    /* Accounts valid */
    if ((acc_from < num_accounts) && (acc_to < num_accounts)) {  

      /* Amount available in from account */
      if (amount <= account_balances[acc_from]) { 
          log_tr(amount, acc_from, acc_to, account_balances[acc_from]); 
          account_balances[acc_from] -= amount;  
          account_balances[acc_to] += amount;  
      } else {
          log_tr_no_funds(amount, acc_from, acc_to, account_balances[acc_from]); 
      }
    } else {
        log_tr_no_acc(amount, acc_from, acc_to); 
    }
}

/*--------------------------------------------------------------------
 * @brief balance 
 *  Return the current balance of account acc_num 
 * @param acc_num: The number of the account 
 * @param balance: The current balance of account acc_num
 */
void acc_balance(int acc_num)
{
    /* Valid account */
    if (acc_num < num_accounts) {
        log_bl(acc_num, account_balances[acc_num]); 
    } else {
        log_bl_no_acc(acc_num); 
    }
}
