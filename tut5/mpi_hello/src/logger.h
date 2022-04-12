/**
  * @file logger.h
  * @description A definition of functions offered by the logger
  */

#ifndef _LOGGER_H
#define _LOGGER_H

#include <mpi.h>

/* Functions */
void log_msg(char *msg);
void log_recv();

#endif
