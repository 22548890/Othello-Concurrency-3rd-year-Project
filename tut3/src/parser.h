/**
  * @file parser.h
  * @description A definition of the structures and functions necessary to read
  *              in the transactions file, parse it, and load it into the  
  *              necessary datastructures.
  */

#ifndef _PARSER_H
#define _PARSER_H

/**
 * @brief Reads in a list of transactions from a specified file and stores the
 * transactions in appropriate data structures 
 * 
 * Reads the transaction.list file, parses it, and loads the transactions into a linked list of transactions. 
 *
 * @param filename The name of the transaction.list file 
 */

struct transaction_t* parse_transaction_file(char* filename);

#endif
