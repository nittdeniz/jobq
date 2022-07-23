#include "config.h"
#include "parse_config.h"
#include "util.h"

#include <netinet/in.h> // struct socket_in
#include <sys/socket.h>

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_PENDING_CONNECTIONS 4

int main(int argc, char** argv){
    struct Config config;
    parse_config(CONFIG_FILE, &config);

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
    socket_address.sin_port = htons(config.port);

    if( bind(socket_descriptor, (struct sockaddr*)&socket_address, sizeof(socket_address)) == -1 ){
        quit_with_error("Bind failure");
    }

    if( listen(socket_descriptor, MAX_PENDING_CONNECTIONS) == -1 ){
        quit_with_error("Listen failure");
    }


    socklen_t socket_address_length = sizeof(socket_address);
    while( true ){
        printf("Waiting 1\n");
        int temp_descriptor = accept(socket_descriptor, (struct sockaddr*)&socket_address, &socket_address_length);

        char buffer[MAX_COMMAND_LENGTH] = {0};
        ssize_t read_bytes = read(temp_descriptor, &buffer, MAX_COMMAND_LENGTH);

        if(read_bytes == -1 ){
            quit_with_error("Read failure.");
        }
        printf("Waiting 2\n");
        sleep(1);
    }
}