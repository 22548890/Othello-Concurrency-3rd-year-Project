/**
  * @file loader.h
  */

#ifndef _LOADER_H
#define _LOADER_H

#define DP_T 0
#define WD_T 1 
#define TR_T 2 
#define BL_T 3 

/**
 */
struct transaction_t {
    /** The type of transaction */
    int type;
    /** The source account */
    int src;
    /** The destination account, if applicable */
    int dest;
    /** The amount, if applicable */
    double amount;
    /** A pointer to the next instruction */
    struct transaction_t *next;
};

/*
 * Creates a deposit transaction and adds 
 * it to a linked list of transactions 
 */
struct transaction_t* load_deposit(int type, int dest, double amount, struct transaction_t* transaction_list); 

/*
 * Creates a withdrawal transaction and adds 
 * it to a linked list of transactions 
 */
struct transaction_t* load_withdrawal(int type, int src, double amount, struct transaction_t* transaction_list); 

/*
 * Creates a transfer transaction and adds 
 * it to a linked list of transactions 
 */
struct transaction_t* load_transfer(int type, int src, int dest, double amount, struct transaction_t* transaction_list); 

/*
 * Creates a balance transaction and adds 
 * it to a linked list of transactions 
 */
struct transaction_t* load_balance(int type, int src, struct transaction_t* transaction_list); 

#endif
