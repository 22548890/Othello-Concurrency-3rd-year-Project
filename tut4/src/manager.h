/**
 * @file manager.h
 */
#ifndef _MANAGER_H
#define _MANAGER_H

#include "loader.h"

void schedule_processes(struct pcb_t *ready_pcbs, struct resource_t *resource_list,  
        int sched_algo, int time_quantum);

#endif
