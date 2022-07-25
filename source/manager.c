#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <malloc.h>

#include "job.h"
#include "manager.h"
#include "util.h"

struct Elem* start_job(struct Manager* m, struct Elem* elem){
    if( m == NULL || elem == NULL || m->running_queue == NULL || m->waiting_queue == NULL ){
        return NULL;
    }
    elem->job.core_mask = 0;
    unsigned long long int it = m->available_cores;
    for( int i = 0, j = 0; i < elem->job.cores; j++, it >>= 1 ){
        if( (it & 1) == 1 ){
            elem->job.core_mask |= (1ULL << j);
            i++;
        }
        //printf("i: %d Available cores: %llX mask: %llX\n", j, it, elem->job.core_mask);
    }
    printf("Available cores: %llX\n", m->available_cores);
    printf("Mask: %llX\n", elem->job.core_mask);
    m->available_cores ^= elem->job.core_mask;
    printf("Available cores: %llX\n", m->available_cores);

    elem->job.start_time = time(NULL);
    elem->job.end_time = elem->job.start_time + elem->job.time_limit;

    elem->job.pid = fork();
    if( elem->job.pid == -1 ){
        fprintf(stderr, "Failed to fork\n");
    }
    else if( elem->job.pid == 0 ){
        if( chdir(elem->job.working_directory) < 0 ){
            quit_with_error("chdir failure");
        }
        if( setegid(elem->job.group_id) < 0 ){
            quit_with_error("setegid failure");
        }
        if( setgid(elem->job.group_id) < 0 ){
            quit_with_error("setgid failure");
        }
        if( setuid(elem->job.user_id) < 0){
            quit_with_error("setuid failure");
        }
        if( seteuid(elem->job.user_id) < 0){
            quit_with_error("seteuid failure");
        }
        char out[24] = {0};
        char err[24] = {0};
        sprintf(&out[0], "out_%ld.txt", elem->job.id);
        sprintf(&err[0], "err_%ld.txt", elem->job.id);
        int fd_out = open(out, O_WRONLY | O_CREAT, S_IRWXU);
        if( fd_out < 0 ){
            quit_with_error("error open stdout");
        }
        int fd_err = open(err, O_WRONLY | O_CREAT, S_IRWXU);
        if( fd_err < 0 ){
            quit_with_error("error open stderr");
        }
        if( dup2(fd_out, STDOUT_FILENO) < 0 ){
            quit_with_error("dup2 failure out");
        }
        if( dup2(fd_err, STDERR_FILENO) < 0 ){
            quit_with_error("dup2 failure err");
        }
        char* command = &elem->job.cmd[0];
//        fprintf(stderr, "%s\n", command);
        int len = 0;
        char** split_str = split(command, ' ', &len);
//        fprintf(stderr, "len: %d\n", len);
        char* arr[] = {"taskset", "-a", NULL};
        int memory_len = snprintf(NULL,0,"0x%llX", elem->job.core_mask);
        arr[2] = malloc(memory_len + 1);
        snprintf(arr[2], memory_len+1,"0x%llX", elem->job.core_mask);
//        fprintf(stderr,"core core_mask: %s\n", arr[2]);
        char* params[3+len];
        memcpy(&params[0], &arr[0], 3 * sizeof(char*));
        memcpy(&params[3], &split_str[0], len * sizeof(char*));
        char** jt = &params[0];
        while( *jt++ != (char*)NULL ){
            fprintf(stderr, "it: %p %s\n", *jt, *jt);
        }
        execv("/usr/bin/taskset", params);
    }
    char buffer[17] = {0};
//    sprintf(&buffer[0], "pid: %ld", (long)elem->job.pid);
    puts(buffer);
    pthread_mutex_lock(m->running_lock);
    push_back(m->running_queue, elem->job);
    pthread_mutex_unlock(m->running_lock);
    struct Elem* next = delete(m->waiting_queue, elem);
    return next;
}

void clear_finished_and_overdue_jobs(struct Manager* m){
    if( m == NULL ){
        return;
    }
    pthread_mutex_lock(m->running_lock);
    struct Elem* elem = m->running_queue->first;
    while( elem != NULL ){
        if( elem->job.end_time < time(NULL) ){
            if( kill(elem->job.pid, 9) < 0 ){
                quit_with_error("Could not kill process");
            }
            printf("Terminated job: %ld\n", (long)elem->job.pid);
        }
        pid_t pid = waitpid(elem->job.pid, NULL, WNOHANG);
//        printf("pid: %ld\n", (long)pid);
        if( pid == elem->job.pid ){
            printf("Job %ld finished.\n", (long)elem->job.pid);
            elem = delete(m->running_queue, elem);
        }else{
            elem = elem->next;
        }
    }
    pthread_mutex_unlock(m->running_lock);
}


time_t get_latest_end_time(struct Manager* m){
    time_t latest = 0;
    pthread_mutex_lock(m->running_lock);
    struct Elem* elem = m->running_queue->first;
    while( elem != NULL ){
        if( elem->job.end_time > latest ){
            latest = elem->job.end_time;
        }
        elem = elem->next;
    }
    pthread_mutex_unlock(m->running_lock);
    return latest;
}

long get_free_cores(struct Manager* m){
    if( m == NULL ){
        return 0;
    }
    long result = 0;
    unsigned long long int n = m->available_cores;
    while( n ){
        result += (long)(n&1ULL);
        n>>=1;
    }
    return result;
}

void start_jobs(struct Manager* m){
    long n_free_cores = get_free_cores(m);
    if( m->priority_elem != NULL && m->priority_elem->job.cores <= n_free_cores ){
        start_job(m, m->priority_elem);
        m->latest_end_time = get_latest_end_time(m);
    }else
    {
        pthread_mutex_lock(m->waiting_lock);
        struct Elem *waiting_elem = m->waiting_queue->first;
        while( waiting_elem != NULL)
        {
            if( waiting_elem == m->priority_elem )
            {
                waiting_elem = waiting_elem->next;
                continue;
            }
            if( m->priority_elem == NULL)
            {
                if( waiting_elem->job.cores <= get_free_cores(m) )
                {
                    waiting_elem = start_job(m, waiting_elem);
                    m->latest_end_time = get_latest_end_time(m);
                    continue;
                }else
                {
                    m->priority_elem = waiting_elem;
                    m->priority_elem->job.start_time = m->latest_end_time;
                }
            }else
            {
                if( waiting_elem->job.time_limit + time(NULL) < m->priority_elem->job.start_time && waiting_elem->job.cores <= get_free_cores(m) )
                {
                    waiting_elem = start_job(m, waiting_elem);
                    continue;
                }
            }
            waiting_elem = waiting_elem->next;
        }
        pthread_mutex_unlock(m->waiting_lock);
    }
}