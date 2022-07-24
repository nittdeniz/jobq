#include "job.h"
#include "queue.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

struct Elem* start_job(struct Elem* elem, struct Job_Queue* running_queue, struct Job_Queue* waiting_queue, int* free_cores){
    if( elem == NULL || running_queue == NULL || waiting_queue == NULL || free_cores == NULL ){
        return NULL;
    }
    elem->job.start_time = time(NULL);
    elem->job.end_time = elem->job.start_time + elem->job.time_limit;

    elem->job.pid = fork();
    if( elem->job.pid == -1 ){
        puts("Failed to execute command.\n");
    }
    else if( elem->job.pid == 0 ){
        chdir(elem->job.working_directory);
        char* command = "ping -c 5 -i 5 localhost";
        //char* args[7] = {"ping", "-c", "5", "-i", "5", "localhost", NULL};
        char* tok = strtok(command, " ");
        while( tok != NULL ){
            tok = strtok(NULL, " ");
        }
        char* arr[] = {"taskset", "-c", "1"};
        char* params = malloc(3 * sizeof(char*) + 6 * sizeof(char*));
        memcpy(params, arr, 3 * sizeof(char*));
        memcpy(params + 3*sizeof(char*), tok, )
        execv("/usr/bin/taskset", arr);
        free(params);
    }
    char buffer[16] = {0};
    sprintf(&buffer[0], "pid: %ld", elem->job.pid);
    puts(buffer);
    push_back(running_queue, elem->job);
//    *free_cores -= elem->job.cores;
    return delete(waiting_queue, elem);
}

time_t get_latest_end_time(struct Job_Queue *queue){
    time_t latest = 0;
    struct Elem* elem = queue->first;
    while( elem != NULL ){
        if( elem->job.end_time > latest ){
            latest = elem->job.end_time;
        }
        elem = elem->next;
    }
    return latest;
}

void *queue(void *pointers){
    void** p = (void**) pointers;
//    char* message_buffer = p[0];
    struct Config* config = p[1];
    struct Job_Queue* waiting_queue = p[2];

    int available_cores[config->maxcores];

    struct Job_Queue running_queue;
    running_queue.first = NULL;
    running_queue.last = NULL;
    struct Elem* priority_job_elem = NULL;
    time_t latest_end_time = get_latest_end_time(&running_queue);
    while( true ){
        long free_cores = config->maxcores;
        struct Elem *queue_elem = running_queue.first;
        while( queue_elem != NULL ){
            free_cores -= queue_elem->job.cores;
            queue_elem = queue_elem->next;
        }
        if( free_cores <= 0 ){
            sleep(1);
            continue;
        }
        if( priority_job_elem != NULL && priority_job_elem->job.cores <= free_cores ){
            start_job(priority_job_elem, &running_queue, waiting_queue, &available_cores[0]);
            latest_end_time = get_latest_end_time(&running_queue);
        }else{
            struct Elem *waiting_elem = waiting_queue->first;
            while( waiting_elem != NULL ){
                if( waiting_elem == priority_job_elem ){
                    waiting_elem = waiting_elem->next;
                    continue;
                }
                if( priority_job_elem == NULL){
                    if( waiting_elem->job.cores <= free_cores ){
                        waiting_elem = start_job(waiting_elem, &running_queue, waiting_queue, &available_cores[0]);
                        latest_end_time = get_latest_end_time(&running_queue);
                        continue;
                    }else{
                        priority_job_elem = waiting_elem;
                        priority_job_elem->job.start_time = latest_end_time;
                    }
                }else{
                    if( waiting_elem->job.time_limit + time(NULL) < priority_job_elem->job.start_time && waiting_elem->job.cores <= free_cores){
                        waiting_elem = start_job(waiting_elem, &running_queue, waiting_queue, &available_cores[0]);
                        continue;
                    }
                }
                waiting_elem = waiting_elem->next;
            }
        }
        sleep(1);
    }
    return NULL;
}