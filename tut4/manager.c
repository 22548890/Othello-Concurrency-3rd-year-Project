/**
 * @file manager.c
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "logger.h"
#include "manager.h"

#define QUANTUM 1
#define TRUE 1
#define FALSE 0

void process_request(struct pcb_t *pcb, struct instruction_t *instruction);
void process_release(struct pcb_t *pcb, struct instruction_t *instruction);
int acquire_resource(struct pcb_t *pcb ,char* resource);

void print_available_resources();
void process_to_readyq(struct pcb_t *proc);
void process_to_waitingq(struct pcb_t *proc);
void process_to_terminatedq(struct pcb_t *proc);

void execute_instruction(struct pcb_t *pcb, struct instruction_t *instruction);

struct pcb_t* detect_deadlock();
void resolve_deadlock (struct pcb_t *pcb);

void print_process_resources(struct pcb_t *pcb);
void print_q (struct pcb_t *pcb);
struct pcb_t* delete_pcb(struct pcb_t *pcb, struct pcb_t *queue);
struct resource_t* delete_resource(struct resource_t *resource, struct resource_t *queue);

/**
 * The following structs are the queues as required by the project spec together
 * with a pointer to the end of the queue to make insertions into the respective
 * queues easy.
 */
struct pcb_t *ready_queue = NULL;
struct pcb_t *waiting_queue = NULL;
struct pcb_t *terminated_queue = NULL;

struct resource_t *global_resource_list = NULL;
/**
 * @brief Schedules each instruction of each process in a round-robin fashion.
 * The number of instruction to execute for each process is governed by the
 * QUANTUM variable.
 *
 * @param pcb The process control block which contains the current process as
 * well as a pointer to the next process control block.
 * @param resource The list of resources available to the system.
 * @param algorithm The type of algorithm to be used (0: Priority, 1: RR).
 * @param time_q The time quantum for the RR algorithm.
 */
void schedule_processes(struct pcb_t *ready_pcbs, struct resource_t *resource_list, 
        int sched_algo, int time_quantum) {

    //TODO: Add ready_pcbs to the ready_queue
    //      Add resource_list to a globally declared resource list 

    struct pcb_t *current_pcb;
    struct pcb_t *priority_pcb;

    ready_queue = ready_pcbs;
    global_resource_list = resource_list;
    current_pcb = ready_queue;

    if (sched_algo == 0) {
        while (ready_queue != NULL) {
            current_pcb = ready_queue;
            priority_pcb = ready_queue;
            while (current_pcb != NULL) {
                if (current_pcb->priority < priority_pcb->priority) {
                    priority_pcb = current_pcb;
                } 
                current_pcb = current_pcb->next;
            }
            while (priority_pcb->next_instruction != NULL) {
                execute_instruction(priority_pcb, priority_pcb->next_instruction);
                if (priority_pcb->state == WAITING) {
                    break;
                }
            }
            if (priority_pcb->next_instruction == NULL) {
                priority_pcb->state = TERMINATED;
                ready_queue = delete_pcb(priority_pcb, ready_queue);
                process_to_terminatedq(priority_pcb); 
            }
        }
    } else if (sched_algo == 1) {
        while (current_pcb != NULL) {
            int i, terminated;
            terminated = FALSE;
            current_pcb = ready_queue;
            if (current_pcb == NULL) {
                break;
            }
            for (i = 0; i < time_quantum; i++) {
                if (current_pcb->next_instruction == NULL) {
                    current_pcb->state = TERMINATED;
                    ready_queue = delete_pcb(current_pcb, ready_queue);
                    process_to_terminatedq(current_pcb);
                    terminated = TRUE;
                    break;
                }
                execute_instruction(current_pcb, current_pcb->next_instruction);
                if (current_pcb->state== WAITING) {
                    break;
                }
                if (current_pcb->next_instruction == NULL) {
                    current_pcb->state = TERMINATED;
                    ready_queue = delete_pcb(current_pcb, ready_queue);
                    process_to_terminatedq(current_pcb);
                    terminated = TRUE;
                    break;
                }
            }
            if (terminated) {
                current_pcb = ready_queue;
                continue;
            }
            if (current_pcb->state <= RUNNING) {
                ready_queue = delete_pcb(current_pcb, ready_queue);
                process_to_readyq(current_pcb);
            }
        }
    }
}

/**
 * @brief Interpret a instruction and calls the respective functions
 *
 * @param pcb The current process for which the instruction must me executed.
 * @param instruction The instruction which must be executed.
 */
void execute_instruction(struct pcb_t *pcb, struct instruction_t *instruction) {

    switch (instruction->type) {

        case REQ_V: 
            process_request(pcb, instruction);
            break;
        
        case REL_V: 
            process_release(pcb, instruction);
            break;

        default:
            break;
    }

}

/**
 * @brief Handles the request resource instruction.
 *
 * Executes the request instruction for the process. The function loops
 * through the list of resources and acquires the resource if it is available.
 * If the resource is not available the process sits in the waiting queue and
 * tries to acquire the resource on the next cycle.
 *
 * @param current The current process for which the resource must be acquired.
 * @param instruct The instruction which requests the resource.
 */
void process_request(struct pcb_t *pcb, struct instruction_t *instruction) {

    char* resource_name;
    int available;

    resource_name = instruction->resource;

    available = acquire_resource(pcb, resource_name);

    if (available) {
        log_request_acquired(pcb->page->name, resource_name);
        printf("%s req %s: acquired; ", pcb->page->name, resource_name);    
        print_available_resources();
        pcb->next_instruction = instruction->next;
        pcb->state = RUNNING;
    } else {
        pcb->state = WAITING;
        log_request_waiting(pcb->page->name, resource_name); 
        printf("%s req %s: waiting;\n", pcb->page->name, resource_name);
    }
}

/**
 * @brief Handles the release resource instruction.
 *
 * Executes the release instruction for the process. If the resource can be
 * released the process is ready for next execution. If the resource can not
 * be released the process sits in the waiting queue.
 *
 * @param current The process which releases the resource.
 * @param instruct The instruction to release the resource.
 */
void process_release(struct pcb_t *pcb, struct instruction_t *instruction) {
    char* resource_name;
    struct resource_t *current_resource;
    struct pcb_t *temp_pcb;

    resource_name = instruction->resource;
    current_resource = pcb->resources;
    
    while (current_resource != NULL && strcmp(current_resource->name, resource_name) != 0) {
        current_resource = current_resource->next;
    }
    if (current_resource == NULL) {
        // resource not assigned to process 
        log_release_error(pcb->page->name, instruction->resource);
        printf("%s rel %s: Error: Nothing to release\n", pcb->page->name, instruction->resource);
        pcb->next_instruction = pcb->next_instruction->next;
    } else { // Resource is in list
        // First get the resource in global list
        current_resource = global_resource_list;
        while (strcmp(current_resource->name, resource_name) != 0 || current_resource->available == TRUE) {
            current_resource = current_resource->next;
        }
        current_resource->available = TRUE;  
        temp_pcb = waiting_queue;
        while (temp_pcb != NULL) {
            if (strcmp(resource_name, temp_pcb->wait_resource) == 0) {
                temp_pcb->wait_resource = NULL;
                waiting_queue = delete_pcb(temp_pcb, waiting_queue);
                process_to_readyq(temp_pcb);
                temp_pcb = waiting_queue;
                continue;
            }
            temp_pcb = temp_pcb->next;
        }
        
        // successful release
        log_release_released(pcb->page->name, instruction->resource);
        printf("%s rel %s: released; ", pcb->page->name, instruction->resource);
        print_available_resources();
        pcb->resources = delete_resource(current_resource, pcb->resources);
        pcb->next_instruction = pcb->next_instruction->next;    
    }
}

/**
 * @brief Acquires the resource specified by resourceName.
 *
 * The function iterates over the list of resources trying to acquire the
 * resource specified by resourceName. If the resources is available, the
 * process acquires the resource. The resource is indicated as not available
 * in the resourceList and 1 is returned indicating that the resource has been
 * acquired successfully.
 *
 * @param resourceName The name of the resource to acquire.
 * @param resources The list of resources.
 * @param p The process which acquires the resource.
 *
 * @return 1 for TRUE if the resource is available. 0 for FALSE if the resource
 * is not available.
 */
int acquire_resource(struct pcb_t *pcb, char* resource_name) {
    struct resource_t *current_resource;
    int available;
    current_resource = global_resource_list;

    available = FALSE;
    while (current_resource != NULL) {
        if (strcmp(current_resource->name, resource_name) == 0 && current_resource->available) {
            available = TRUE;
            break;
        }
        current_resource = current_resource->next;
    }
    if (available) {
        //add just the name to list of resources
        struct resource_t *new;
        new = malloc(sizeof(struct resource_t));
        new->name = current_resource->name;
        new->next = NULL;

        if (pcb->resources == NULL) {
            pcb->resources = new;
        } else {
            new->next = pcb->resources;
            pcb->resources = new;
        }
        current_resource->available = FALSE;
        return TRUE;
    } else {
        ready_queue = delete_pcb(pcb, ready_queue);
        process_to_waitingq(pcb);
        pcb->wait_resource = resource_name;
        return FALSE;
    }
}

/**
 * @brief Add process (with pcb proc) to the readyQueue 
 *
 * @param proc The process which must be set to ready.
 */
void process_to_readyq(struct pcb_t *proc) {
    struct pcb_t *temp;
    temp = ready_queue;
    proc->state = READY;
    proc->next = NULL;
    if (ready_queue == NULL) {
        ready_queue = proc;
    } else {
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = proc;
    }
    log_ready(proc->page->name, ready_queue);
}

/**
 * @brief Add process (with pcb proc) to the waitingQueue 
 *
 * @param proc The process which must be set to waiting.
 */
void process_to_waitingq(struct pcb_t *proc) {
    struct pcb_t *temp;
    temp = waiting_queue;
    proc->state = WAITING;
    proc->next = NULL;
    if (waiting_queue == NULL) {
        waiting_queue = proc;
    } else {
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = proc;
    }
    log_waiting(proc->page->name, waiting_queue);
}

/**
 * @brief Add process with pcb proc to the terminatedQueue 
 *
 * @param proc The process which has terminated 
 */
void process_to_terminatedq(struct pcb_t *proc) {
    struct pcb_t *temp;
    temp = terminated_queue;
    proc->state = TERMINATED;
    proc->next = NULL;
    if (terminated_queue == NULL) {
        terminated_queue = proc;
    } else {
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = proc;
    }
    log_terminated(proc->page->name);
    printf("%s terminated\n", proc->page->name);
}

/**
 * @brief Takes the waiting queue and detects deadlock
 */
struct pcb_t* detect_deadlock() {
    printf("detect_deadlock not implemented\n");

    // if deadlock detected
    log_deadlock_detected();

    return NULL;
}

/**
 * @brief Releases a processes' resources and sets it to its first instruction.
 *
 * Generates release instructions for each of the processes' resoures and forces
 * it to execute those instructions.
 *
 * @param pcb The process chosen to be reset and release all of its resources.
 *
 */
void resolve_deadlock (struct pcb_t *pcb) {
    printf("resolve_deadlock not implemented\n");
}

/**
 * @brief Prints the global list of available resources.
 */
void print_available_resources() {

    struct resource_t *current_resource;
    current_resource = get_available_resources();

    printf("Available:");
    do {

        if (current_resource->available) {
            printf(" %s", current_resource->name);
        }
        current_resource = current_resource->next;

    } while (current_resource != NULL);

    printf("\n");
}

struct pcb_t* delete_pcb(struct pcb_t *pcb, struct pcb_t *queue) {
    // Delete this pcb out of queue
    struct pcb_t *current_pcb, *prev;
    current_pcb = queue;

    if (current_pcb != NULL && strcmp(current_pcb->page->name, pcb->page->name) == 0) {
        queue = queue->next;
        return queue;
    }
    while (current_pcb != NULL && strcmp(current_pcb->page->name, pcb->page->name) != 0) {
        prev = current_pcb;
        current_pcb = current_pcb->next;
    }
    if (current_pcb == NULL) {
        return queue;
    }
    prev->next = current_pcb->next;
    return queue;
    // Delete
}

struct resource_t* delete_resource(struct resource_t *resource, struct resource_t *queue) {
    // Delete this pcb out of queue
    struct resource_t *current_resource, *prev;
    current_resource = queue;

    if (current_resource != NULL && strcmp(current_resource->name, resource->name) == 0) {
        queue = current_resource->next;
        return queue;
    }
    while (current_resource != NULL && strcmp(current_resource->name, resource->name) != 0) {
        prev = current_resource;
        current_resource = current_resource->next;
    }
    if (current_resource == NULL) {
        return queue;
    }
    prev->next = current_resource->next;
    return queue;
    // Delete
}

#ifdef DEBUG
void print_process_resources(struct pcb_t *pcb) {
    struct resource_t *resource;
    resource = pcb->resources;

    printf("Resources for pcb %s: ",pcb->page->name);
    while (resource != NULL) {
        printf("%s->", resource->name);
        resource = resource->assigned_to;
    }
    printf("\n");
}

void print_q (struct pcb_t *pcb) {
    while (pcb != NULL) {
        printf(" %s,", pcb->page->name);
        pcb = pcb->q_next;
    }
    printf("\n");
}
#endif

void dealloc_queues()
{       
    dealloc_pcb_list(*ready_queue);
    dealloc_pcb_list(*waiting_queue);
    dealloc_pcb_list(*terminated_queue);
}
