#include "colours.h"
#include "config.h"
#include "job.h"
#include "manager.h"
#include "parse_config.h"
#include "queue.h"
#include "server.h"
#include "time.h"
#include "util.h"

#include <netinet/in.h> // struct socket_in
#include <sys/socket.h>

#include <malloc.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void* server(void* pointers){
    void** p = (void**) pointers;
//    char* message_buffer = p[0];
    struct Config* config = p[1];
    struct Manager* m = p[2];
    int socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if( socket_descriptor == -1 ){
        quit_with_error("Socket failure");
    }

    struct sockaddr_in socket_address;
    int option = 1;
    if( setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option))) {
        quit_with_error("Setsockopt failure");
    }
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = INADDR_ANY;
    socket_address.sin_port = htons(config->port);

    if( bind(socket_descriptor, (struct sockaddr*)&socket_address, sizeof(socket_address)) == -1 ){
        quit_with_error("Bind failure");
    }

    if( listen(socket_descriptor, MAX_PENDING_CONNECTIONS) == -1 ){
        quit_with_error("Listen failure");
    }


    socklen_t socket_address_length = sizeof(socket_address);


    long job_id = 0;
    while( true ){
        int temp_descriptor = accept(socket_descriptor, (struct sockaddr*)&socket_address, &socket_address_length);

        char buffer[sizeof(struct Job)+1] = {0};
        ssize_t read_bytes = read(temp_descriptor, &buffer, sizeof(struct Job));

        if(read_bytes < 0 ){
            quit_with_error("Read failure.");
        }
        if( read_bytes == 0 ){
            continue;
        }
        if( buffer[0] == MSG_SUBMIT ){
            struct Job job;
            memcpy(&job, &buffer[1], sizeof(struct Job));
            job.id = job_id++;
            pthread_mutex_lock(m->waiting_lock);
            push_back(m->waiting_queue, job);
            pthread_mutex_unlock(m->waiting_lock);
            char answer_buffer[ANSWER_BUFFER] = {0};
            sprintf(&answer_buffer[0], "Job submitted. Id: %ld\nEnter jobq status to check status.\n", job.id);
            send(temp_descriptor, answer_buffer, strlen(answer_buffer), 0);
        }
        if( buffer[0] == MSG_STOP ){
            printf("STOP!\n");
        }
        if( buffer[0] == MSG_STATUS ){
            char answer_buffer[ANSWER_BUFFER] = {0};
            int n = 0;
            n += snprintf(&answer_buffer[n], ANSWER_BUFFER-n, "available cores: %ld\n", get_free_cores(m));
            n += snprintf(&answer_buffer[n], ANSWER_BUFFER-n, "job id\tstatus\t\tuser\t\tcores\tstart\t\tend\t\tcommand\n");
            pthread_mutex_lock(m->running_lock);
            struct Elem* running_element = m->running_queue->first;
            while( running_element != NULL ){
                struct Job j = running_element->job;
                char start_time[20] = {0};
                char end_time[20] = {0};
                strftime(&start_time[0], 15, "%d-%m %H:%M:%S", localtime(&j.start_time));
                strftime(&end_time[0], 15, "%d-%m %H:%M:%S", localtime(&j.end_time));
                n += snprintf(&answer_buffer[n], ANSWER_BUFFER-n,"%ld\t%s[running]%s\t%s\t%ld\t%s\t%s\t%s\n", (long)j.id, GREEN, RESET, j.user_name, j.cores, &start_time[0], &end_time[0], &j.cmd[0]);
                running_element = running_element->next;
            }
            pthread_mutex_unlock(m->running_lock);
            pthread_mutex_lock(m->waiting_lock);
            struct Elem* waiting_elem = m->waiting_queue->first;
            while( waiting_elem != NULL ){
                struct Job j = waiting_elem->job;
                if( waiting_elem == m->priority_elem ){
                    char start_time[20] = {0};
                    strftime(&start_time[0], 15, "%d-%m %H:%M:%S", localtime(&j.start_time));
                    n += snprintf(&answer_buffer[n], ANSWER_BUFFER - n, "%ld\t%s[priority]%s\t%s\t%ld\t%s\t%s\t%s\n", (long) j.id, CYAN, RESET, j.user_name,
                                  j.cores, &start_time[0], "n/a\t", &j.cmd[0]);
                }else{
                    n += snprintf(&answer_buffer[n], ANSWER_BUFFER - n, "%ld\t%s[waiting]%s\t%s\t%ld\t%s\t%s\t%s\n", (long) j.id, RED, RESET, j.user_name,
                                  j.cores, "n/a\t", "n/a\t", &j.cmd[0]);
                }
                waiting_elem = waiting_elem->next;
            }
            pthread_mutex_unlock(m->waiting_lock);

            send(temp_descriptor, answer_buffer, strlen(answer_buffer), 0);
        }

        sleep(1);
    }
    return NULL;
}