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

void* server(void* pointers){
    void** p = (void**) pointers;
//    char* message_buffer = p[0];
    struct Config* config = p[1];
    struct Job_Queue* waiting_queue = p[2];
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
            push_back(waiting_queue, job);
            puts("Job submitted.\n");
        }
        if( buffer[0] == MSG_STOP ){
            printf("STOP!\n");
        }
        if( buffer[0] == MSG_STATUS ){
            printf("STATUS\n");
        }
        sleep(1);
    }
    return NULL;
}