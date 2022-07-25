#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv){
    pid_t pid = fork();
    if( pid == 0 )
    {
        if( chdir("/test") < 0 )
        {
            perror("chdir failure");
        }
        if( seteuid(1001))
        {
            perror("Seteuid failure");
        }
        char out[24] = {0};
        char err[24] = {0};
        sprintf(&out[0], "out_%ld.txt", 12L);
        sprintf(&err[0], "err_%ld.txt", 12L);
        int fd_out = open(&out[0], O_WRONLY | O_CREAT, S_IRWXU);
        if( fd_out < 0 )
        {
            perror("open out");
        }
        int fd_err = open(&err[0], O_WRONLY | O_CREAT, S_IRWXU);
        if( fd_err < 0 )
        {
            perror("error out");
        }
        if( dup2(fd_out, STDOUT_FILENO) < 0 )
        {
            perror("dup2 out failure");
            printf("%s\n", out);
        }
        if( dup2(fd_err, STDERR_FILENO) < 0 )
        {
            perror("dup2 err failure");
            printf("%s\n", err);
        }
        fprintf(stderr, "user_id: %ld\n", (long) geteuid());
        char *command[7] = {"ping", "-c", "5", "-i", "1", "localhost", (char *) NULL};
        char *taskset[3] = {"taskset", "-c", "1"};
        char *params[10];
        memcpy(&params[0], &taskset[0], 3 * sizeof(char *));
        memcpy(&params[3], &command[0], 7 * sizeof(char *));
        execv("/usr/bin/taskset", params);
    }
    printf("done\n");
}