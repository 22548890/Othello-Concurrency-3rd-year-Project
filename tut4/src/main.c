/**
 * @mainpage Process Simulation
 *
 * The project consists of 3 main files parser, loader and manager. Which 
 * respectively handles the loading, parsing and management of the processes.
 *
 * @section make_sec Compile
 * 
 * $ make
 *
 * @section run_sec Execute
 *
 * $ ./process-management data/process.list 1 4
 * 
 */

#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "loader.h"
#include "manager.h"

void print_ready_queue(struct pcb_t *pcb);
void print_instruction(struct instruction_t *instr);

int main(int argc, char** argv) {
    char* filename;
    struct pcb_t *ready_pcbs;
    struct resource_t *resource_list;
    int sched_algo = 0;
    int time_quantum = 1;
    filename = NULL;

    if (argc < 1) {
        printf("Usage: ./process_manager <datafile> <scheduler: 1 Priority 2 RR> <Time quantum if RR>\n");
        return EXIT_FAILURE; 
    }

    if (argc > 2) {
        sched_algo = strtol(argv[2], NULL, 10);
        #ifdef DEBUG
        printf("Scheduling sched_algo: %d\n", algorithm);
        #endif
        if (argc == 4) {
            time_quantum = strtol(argv[3], NULL, 10);
            #ifdef DEBUG
            printf("Time quantum: %d\n", time_quantum);
            #endif
        }
    }

    filename = argv[1];
    parse_process_file(filename);

    ready_pcbs = get_loaded_pcbs();
    resource_list = get_available_resources();

    #ifdef DEBUG
    print_ready_queue(ready_pcbs);
    #endif

    /* schedule the process using sched_algo, and if RR time_quantum */
    schedule_processes(ready_pcbs, resource_list, sched_algo, time_quantum);
    
    dealloc_data_structures();
    
    return EXIT_SUCCESS;
}

#ifdef DEBUG
void print_ready_queue ( struct pcb_t *pcb ) {
    struct pcb_t *current_pcb;
    current_pcb = pcb;
    printf("pcb full list:\n");
    do {
        printf("%s (%d) priority %d:\n", current_pcb->page->name, \
             current_pcb->state, current_pcb->priority);
        print_instruction(current_pcb->next_instruction);
        current_pcb = current_pcb->next;
    } while(current_pcb != NULL);
    printf("\n");
}

void print_instruction(struct instruction_t *instr) {
    while (instr != NULL) {
        switch (instr->type) {
        case REQ_V: 
            printf("(req %s)\n", instr->resource);
            break;
        case REL_V: 
            printf("(rel %s)\n", instr->resource);
            break;
        case SEND_V :
            printf("(send %s %s)\n", instr->resource, instr->msg);
            break;
        case RECV_V:
            printf("(recv %s %s)\n", instr->resource, instr->msg);
            break;
        }
        instr = instr->next;
    }
}

#endif
    
