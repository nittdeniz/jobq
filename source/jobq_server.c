#include "config.h"
#include "job.h"
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
    char* message_buffer = malloc(sizeof(struct Job));
    struct Config *config = malloc(sizeof(struct Config));
    parse_config(CONFIG_FILE, config);
    struct Job_Queue *waiting_queue = malloc(sizeof(struct Job_Queue));

    void* pointers[3] = {(void*) message_buffer, (void*) config, (void*) waiting_queue};

    pthread_t socket_thread_id, queue_thread_id;
    pthread_create(&socket_thread_id, NULL, server, &pointers[0]);
    pthread_create(&queue_thread_id, NULL, queue, &pointers[0]);

    pthread_join(socket_thread_id, NULL);
    pthread_join(queue_thread_id, NULL);

    free(message_buffer);
    free(config);
    free(waiting_queue);
}