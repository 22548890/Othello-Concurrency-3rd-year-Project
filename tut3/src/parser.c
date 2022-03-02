/**
 * @file parser.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "syntax.h"
#include "parser.h"
#include "loader.h"

#define NEW_LINE 0 
#define READING 1 
#define END_OF_FILE 2

FILE* open_transaction_file(char* filename);
struct transaction_t* read_transactions(FILE* fptr);
void read_deposit(FILE* fptr, char *line, int* acc_dest, double* amount);
void read_withdrawal(FILE* fptr, char *line,int* acc_src, double* amount);
void read_transfer(FILE* fptr, char *line, int* acc_src, int* acc_dest, double* amount);
void read_balance(FILE* fptr, char *line, int* acc_src);
int read_string(FILE* fptr, char* line);

/**
 * @brief Reads in a list of transactions from a specified file and stores the
 * transactions in appropriate data structures 
 * 
 * Reads the transaction.list file, parses it, and loads the transactions into a linked list of transactions. 
 *
 * @param filename The name of the transaction.list file 
 */
struct transaction_t* parse_transaction_file(char* filename) {
    FILE *fptr = NULL;
    struct transaction_t* transaction_list = NULL;

    fptr = open_transaction_file(filename);

    if (fptr == NULL) {
        printf("File %s could not be opened\n", filename);
    } else {
        transaction_list = read_transactions(fptr);
        fclose(fptr);
    } 
    return transaction_list;
}

/**
 * @brief Opens the file with filename and returns a pointer to the file.
 *
 * Opens a file, with filename, for read-only. If the file could not be
 * opened return NULL else return the pointer to the file.
 *
 * @param filename The name of the file to open
 *
 * @return A file pointer
 */
FILE* open_transaction_file(char* filename) {
    FILE *file = fopen(filename, "r"); 

    if (file == NULL) {
        file = NULL; 
    }

    return file;
}


/**
 * @brief Reads all the transactions and loads them
 *
 * @param fptr A pointer to the file from which to read.
 * @param line A pointer to a string read from file.
 *
 * @return transaction_list Returns the list of transactions 
 */
struct transaction_t* read_transactions(FILE* fptr) {
    char* trans_type;
    int acc_src, acc_dest;
    double amount;
    int status;
    struct transaction_t* transaction_list = NULL;

    status = NEW_LINE; 
    
    /* reads the transaction name */
    trans_type = malloc(sizeof(char)*64); 
    while((status = read_string(fptr, trans_type)) != NEW_LINE && status != END_OF_FILE){
    if (strcmp(trans_type, DP) == 0) {
        /* Reads the deposit account and amount */
        read_deposit(fptr, trans_type, &acc_dest, &amount);
        transaction_list = load_deposit(DP_T,  acc_dest, amount, transaction_list);
    }
    else if (strcmp(trans_type, WD) == 0) {
        /* Reads the withdrawal account and amount */
        read_withdrawal(fptr, trans_type, &acc_src, &amount);
        transaction_list = load_withdrawal(WD_T,  acc_src, amount, transaction_list);
    }
    else if (strcmp(trans_type, TR) == 0) {
        /* Reads the src and destination accounts and amount */
        read_transfer(fptr, trans_type, &acc_src, &acc_dest, &amount);
        transaction_list = load_transfer(TR_T,  acc_src, acc_dest, amount, transaction_list);
    }
    else if (strcmp(trans_type, BL) == 0) {
        /* Reads the account number */
        read_balance(fptr, trans_type, &acc_src);
        transaction_list = load_balance(BL_T, acc_src, transaction_list);
    }
    else {
        /* Executes on white spaces */
        /* Executes the while loop when encoutering new lines and white spaces, */
        /* Exits the loop when end of file reached */
        if (strcmp(trans_type, "") != 0) {
            printf("\033[22;31mError: uknown transaction type: %s\n\033[0m", trans_type);
            break;
        }
    }
    trans_type = malloc(sizeof(char)*64);
    }
    return transaction_list;
}

/**
 * @brief Reads the account number and amount of a deposit instruction 
 *
 * Uses the read_string function 
 *
 * @param fptr A pointer to the file from which to read.
 * @param acc_dest A pointer to an integer read from the file 
 */
void read_deposit(FILE* fptr, char* line, int* acc_dest, double* amount) {
    read_string(fptr, line); 
    *acc_dest = strtol(line, NULL, 10);
    read_string(fptr, line);
    *amount = strtod(line, NULL);
#ifdef DEBUG
    printf("deposit %d %f \n", *acc_dest, *amount);
#endif
}

/**
 * @brief Reads the account number and amount of a withdrawal instruction 
 *
 * Uses the read_string function
 *
 * @param fptr A pointer to the file from which to read.
 * @param acc_src A pointer to an integer read from the file 
 */
void read_withdrawal(FILE* fptr, char* line, int* acc_src, double* amount) {
    read_string(fptr, line); 
    *acc_src = strtol(line, NULL, 10);
    read_string(fptr, line);
    *amount = strtod(line, NULL);
#ifdef DEBUG
    printf("withdraw %d %f \n", *acc_src, *amount);
#endif
}


/**
 * @brief Reads the account numbers and amount of a transfer instruction 
 *
 * Uses the read_string function 
 *
 * @param fptr A pointer to the file from which to read.
 * @param acc_src A pointer to an integer read from the file 
 * @param acc_dest A pointer to an integer read from the file 
 * @param amount A pointer to a double read from the file 
 */
void read_transfer(FILE* fptr, char* line, int* acc_src, int* acc_dest, double* amount) {
    read_string(fptr, line); 
    *acc_src = strtol(line, NULL, 10);
    read_string(fptr, line);
    *acc_dest = strtol(line, NULL, 10);
    read_string(fptr, line);
    *amount = strtod(line, NULL);
#ifdef DEBUG
    printf("transfer %d %d %f \n", *acc_src, *acc_dest, *amount);
#endif
}

/**
 * @brief Reads the account number of a balance instruction 
 *
 * Uses the read_string function to read the string and convert it to
 * a long int 
 *
 * @param fptr A pointer to the file from which to read.
 * @param line A pointer to a string read from the file 
 * @param acc_dest A pointer to an integer read from the file 
 */
void read_balance(FILE* fptr, char *line, int* acc_src) {
    read_string(fptr, line); 
    *acc_src = strtol(line, NULL, 10);
#ifdef DEBUG
    printf("balance %d \n", *acc_src);
#endif
}

/**
 * @brief Reads the next string.
 *
 * Reads the file character for character and constructs a string until a white
 * space or termination character is matched.
 *
 * @param fptr A pointer to the file from which to read.
 * @param line A pointer to memory where the string can be stored.
 *
 * return status The status indicates when the END_OF_FILE or NEW LINE has 
 * been reached.
 */
int read_string(FILE* fptr, char* line) {
    int index = 0;
    int ch = 0;
    int status = READING;
    
    ch = fgetc(fptr);
    while (ch != '\n' && ch != ' ') {
        if (ch == EOF) {
            status = END_OF_FILE;
            break;
        }
        line[index] = ch;
        index++;
        ch = fgetc(fptr);
        status = ( ch == '\n' ? NEW_LINE : READING );
    }
    line[index] = '\0';

    return status;
}

