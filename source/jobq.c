#include "config.h"
#include "job.h"
#include "parse_config.h"
#include "util.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>


#define CMD_SUBMIT "submit"
#define CMD_STOP "stop"
#define CMD_STATUS "status"



void print_usage(){
    printf("jobq usage:\n");
    printf("jobq %s <n_cores> <time_limit in seconds> \"<command>\"\n", CMD_SUBMIT);
    printf("jobq %s <job_id>\n", CMD_STOP);
    printf("jobq %s\n", CMD_STATUS);
    printf("Please note that the quotation marks are mandatory, if you want to supply arguments to your executable.\n");
}

void prepare_submit_message(char* message_buffer, const char** argv, struct Config* config){
    struct Job job;
    job.id = 0;
    job.pid = 0;
    job.user_id = geteuid();
    job.group_id = getegid();
    job.start_time = 0;
    job.end_time = 0;
    job.core_mask = 0;
    memset(&job.user_name, 0, USERNAME_BUFFER);
    memset(&job.working_directory, 0, DIRECTORY_BUFFER);
    memset(&job.cmd[0], 0, MAX_CMD_LENGTH);
    struct passwd *pass = getpwuid(getuid());
    if( pass == NULL ){
        quit_with_error("getpwuid failure");
    }
    memcpy(&job.user_name[0], (pass->pw_name), strlen(pass->pw_name));

    if( getcwd(&job.working_directory[0], DIRECTORY_BUFFER) == NULL ){
        printf("Working directory path must not exceed %i characters.", DIRECTORY_BUFFER);
        exit(EXIT_FAILURE);
    }

    if( !parse_long(argv[2], &job.cores) ){
        printf("Number of cores must be valid integer: %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }
    if( job.cores < 1 || job.cores > config->maxcores ){
        printf("Number of cores must be 0 < n < %ld. Given: %ld\n", config->maxcores, job.cores);
        exit(EXIT_FAILURE);
    }
    if( !parse_long(argv[3], &job.time_limit) ){
        printf("Time limit must be valid integer: %s\n", argv[3]);
        exit(EXIT_FAILURE);
    }
    if( job.time_limit < 1 || job.time_limit > config->maxtime ){
        printf("Time limit must be 0 < n < %ld. Given: %ld\n", config->maxtime, job.time_limit);
        exit(EXIT_FAILURE);
    }
    if( job.time_limit > config->longjob && job.cores > config->longmaxcores ){
        printf("Jobs taking longer than %ld must not acquire more than %ld cores.\n", config->longjob, config->longjob);
        exit(EXIT_FAILURE);
    }
    size_t cmd_length = strlen(argv[4]);
    if( cmd_length < 1 || cmd_length > MAX_CMD_LENGTH){
        printf("Command length must be between 1 < n < %d. Given: %lu", MAX_CMD_LENGTH, cmd_length);
        exit(EXIT_FAILURE);
    }
    memcpy(&job.cmd, argv[4], cmd_length);
    message_buffer[0] = MSG_SUBMIT;
    memcpy(&message_buffer[1], &job, sizeof(struct Job));
}

void prepare_status_message(char* message_buffer){
    message_buffer[0] = MSG_STATUS;
}

void prepare_stop_message(char* message_buffer, const char** argv){
    struct Job job;
    job.id = 0;
    job.pid = 0;
    job.user_id = geteuid();
    job.group_id = getegid();
    job.start_time = 0;
    job.end_time = 0;
    job.core_mask = 0;
    job.cores = 0;
    job.time_limit = 0;
    memset(&job.user_name, 0, USERNAME_BUFFER);
    memset(&job.working_directory, 0, DIRECTORY_BUFFER);
    memset(&job.cmd[0], 0, MAX_CMD_LENGTH);
    struct passwd *pass = getpwuid(getuid());
    if( pass == NULL ){
        quit_with_error("getpwuid failure");
    }
    memcpy(&job.user_name[0], (pass->pw_name), strlen(pass->pw_name));
    if( !parse_long(argv[2], &job.id) ){
        printf("jobq_id must be valid integer. Given: %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }
    message_buffer[0] = MSG_STOP;
    memcpy(&message_buffer[1], &job, sizeof(struct Job));
}

enum Action{
    SUBMIT = 0,
    STOP = 1,
    STATUS = 2
};

int main(int argc, char const* argv[]){
    if( argc <= 1 ){
        print_usage();
        exit(EXIT_SUCCESS);
    }

    enum Action action;

    if( strcmp(argv[1], CMD_SUBMIT) == 0 && argc == 5 ){
        action = SUBMIT;
    }
    else if( strcmp(argv[1], CMD_STOP) == 0 && argc == 3 ){
        action = STOP;
    }
    else if( strcmp(argv[1], CMD_STATUS) == 0 && argc == 2){
        action = STATUS;
    }
    else{
        printf("Error: invalid arguments.\n");
        print_usage();
        exit(EXIT_FAILURE);
    }

    struct Config config;
    parse_config(CONFIG_FILE, &config);

    int socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if( socket_descriptor == -1 ){
        quit_with_error("Socket failure");
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(config.port);

    int inet = inet_pton(AF_INET, config.server_ip, &server_address.sin_addr);

    if( inet == 0 ){
        printf("%s is not a valid server ip.\n", config.server_ip);
        exit(EXIT_FAILURE);
    }
    else if( inet == -1 ){
        quit_with_error("Inet_pton failure");
    }

    int client_descriptor = connect(socket_descriptor, (struct sockaddr*)&server_address, sizeof(server_address));
    if( client_descriptor == -1 ){
        quit_with_error("Connect failure");
    }

    char message_buffer[sizeof(struct Job)] = {0};

    switch( action ){
        case SUBMIT:{
            prepare_submit_message(&message_buffer[0], argv, &config);
            break;
        }
        case STATUS:{
            prepare_status_message(&message_buffer[0]);
            break;
        }
        case STOP:{
            prepare_stop_message(&message_buffer[0], argv);
            break;
        }
        default:{
        }
    }

    ssize_t result = send(socket_descriptor, message_buffer, sizeof(struct Job), 0);
    if( result == -1 ){
        quit_with_error("Send failure");
    }

    char answer_buffer[ANSWER_BUFFER] = {0};
    size_t answer = read(socket_descriptor, answer_buffer, ANSWER_BUFFER);

    if( answer == -1 ){
        quit_with_error("Read failure");
    }

    puts(answer_buffer);

    // closing the connected socket
    close(client_descriptor);
    close(socket_descriptor);
    return 0;
}