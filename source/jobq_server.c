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

#define MAX_PENDING_CONNECTIONS 4

int main(int argc, char** argv){
    char* message_buffer[sizeof(struct Job)+1];
    struct Config config;
    parse_config(CONFIG_FILE, &config);
    struct Job_Queue waiting_queue;
    waiting_queue.first = NULL;
    waiting_queue.last = NULL;
    struct Job_Queue running_queue;
    running_queue.first = NULL;
    running_queue.last = NULL;

    bool available_cores[config.maxcores];
    for( int i = 0; i < config.maxcores; ++i ){
        available_cores[i] = true;
    }

    pthread_mutex_t running_lock;
    pthread_mutex_t waiting_lock;

    if( pthread_mutex_init(&running_lock, NULL) != 0){
        quit_with_error("Could not initialize mutex");
    }
    if( pthread_mutex_init(&waiting_lock, NULL) != 0){
        quit_with_error("Could not initialize mutex");
    }

    struct Manager manager;
    manager.running_queue = &running_queue;
    manager.waiting_queue = &waiting_queue;
    manager.priority_elem = NULL;
    manager.n_cores = config.maxcores;
    manager.latest_end_time = 0;
    manager.available_cores = &available_cores[0];
    manager.running_lock = &running_lock;
    manager.waiting_lock = &waiting_lock;


    void* pointers[4] = {(void*) message_buffer, (void*) &config, (void*) &manager};


    pthread_t socket_thread_id, queue_thread_id;
    pthread_create(&socket_thread_id, NULL, server, &pointers[0]);
    pthread_create(&queue_thread_id, NULL, queue, &pointers[0]);

    pthread_join(socket_thread_id, NULL);
    pthread_join(queue_thread_id, NULL);
}