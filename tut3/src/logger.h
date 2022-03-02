/**
  * @file logger.h
  * @description A definition of functions offered by the logger
  */

#ifndef _LOGGER_H
#define _LOGGER_H

/* Functions */
void log_dp(double amount, int acc_to , double balance);
void log_dp_no_acc(double amount, int acc_to);
void log_wd(double amount, int acc_from, double balance);
void log_wd_no_funds(double amount, int acc_from, double balance);
void log_wd_no_acc(double amount, int acc_from);
void log_tr(double amount, int acc_from, int acc_to, double balance);
void log_tr_no_funds(double amount, int acc_from, int acc_to, double balance);
void log_tr_no_acc(double amount, int acc_from, int acc_to);
void log_bl(int acc_from, double balance); 
void log_bl_no_acc(int acc_from); 

void log_parallel();
void log_barrier();
void log_critical();
void log_lock(int acc_num);
void log_unlock(int acc_num);

#endif
