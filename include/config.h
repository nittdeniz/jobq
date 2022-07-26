#ifndef JOBQ_CONFIG_H
#define JOBQ_CONFIG_H
#define CONFIG_FILE "/etc/jobq/.config"
#define DIRECTORY_BUFFER 256
#define USERNAME_BUFFER 32
#define NUM_CORES_BUFFER 10
#define TIMELIMIT_BUFFER 10
#define JOBQ_ID_BUFFER 10
#define ANSWER_BUFFER 4096
#define MAX_CMD_LENGTH 512
#define MAX_PENDING_CONNECTIONS 4
#define MSG_SUBMIT 'x'
#define MSG_STOP 'y'
#define MSG_STATUS 'z'

struct Config{
    long port;
    char server_ip[40];
    long maxcores;
    long maxtime;
    long longjob;
    long longmaxcores;
};
#endif //JOBQ_CONFIG_H
