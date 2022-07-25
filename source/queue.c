#include "job.h"
#include "queue.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <util.h>
#include <fcntl.h>

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
        if( seteuid(elem->job.user_id) ){
        	quit_with_error("Seteuid failure");
        }
        setegid(elem->job.group_id);
        char out[24] = {0};
	    char err[24] = {0};
        sprintf(&out[0], "out_%ld.txt", elem->job.id);
	    sprintf(&err[0], "err_%ld.txt", elem->job.id);
	    int fd_out = open(out, O_WRONLY | O_CREAT, S_IRWXU);
	    int fd_err = open(err, O_WRONLY | O_CREAT, S_IRWXU);
	    dup2(fd_out, STDOUT_FILENO);
	    dup2(fd_err, STDERR_FILENO);
	    fprintf(stderr, "user_id: %ld\n",(long)elem->job.user_id);
	    fprintf(stderr, "user_id: %ld\n",(long)geteuid());
        char command[] = "ping -c 5 -i 5 localhost";
        int len = 0;
        char** split_str = split(command, ' ', &len);
        char* arr[] = {"taskset", "-c", "1"};
        char* params[3+len];
        memcpy(&params[0], &arr[0], 3 * sizeof(char*));
        memcpy(&params[3], &split_str[0], len * sizeof(char*));
        execv("/usr/bin/taskset", params);
        free(params);
    }
    char buffer[16] = {0};
    sprintf(&buffer[0], "pid: %ld", (long)elem->job.pid);
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