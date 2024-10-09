#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80 //maximum command length

void execute_command(char *args[], int background) {
    pid_t pid = fork();

    if (pid < 0) {
        //error in creating process
        fprintf(stderr, "Fork failed!\n");
        exit(1);
    } else if (pid == 0) {
        //child process
        //input/output redirection handling
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], "<") == 0) {
                //input redirection
                int fd = open(args[i+1], O_RDONLY);
                if (fd < 0) {
                    perror("Open input file failed!");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
                args[i] = NULL; // remove from args
            } else if (strcmp(args[i], ">") == 0) {
                //output redirection
                int fd = open(args[i+1], O_CREAT | O_WRONLY, 0644);
                if (fd < 0) {
                    perror("Open output file failed!");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                args[i] = NULL; // remove from args
            }
        }
        printf("Executing: %s\n", args[0]);

        //execute the command
        if (execvp(args[0], args) == -1) {
            perror("Execution failed!");
        }
        exit(0);
    } else {
        //parent process
        if (background == 0) {
            //wait for child process to complete if not in background
            wait(NULL);
        }
    }
}

void parse_command(char *command, char *args[], int *background) {
    int i = 0;
    char *token;

    //background flag reset
    *background = 0;

    //tokenizing inputs by spaces
    token = strtok(command, "\n");
    //walking through other tokens
    while (token != NULL) {
        //check if token is '&' background processes
        if (strcmp(token, "&") == 0) {
            *background = 1; // mark command as background
        } else {
            args[i++] = token; //add token to ARGS array
        }
        token = strtok(NULL, "\n");
    }
    //NULL terminate the array of arguments
    args[i] = NULL;
    // if there's no '&', background is false
    if (*background  == 0) {
        *background = 0;
    }
}

int main(void) {
    char *args[MAX_LINE / 2 + 1]; //command line arguments
    char command[MAX_LINE]; //input command
    int background; //background process flag

    while (1) {
        printf("myshell> "); //displaying shell promt
        fflush(stdout);

        //reading input command
        if (fgets(command, MAX_LINE, stdin) == NULL) {
            break;
        }

        //"exit" will terminate the shell
        if (strcmp(command, "exit\n") == 0) {
            break;
        }

        //parsing command to extract arguments and check for background
        parse_command(command, args, &background);
        //executing command
        execute_command(args, background);
    }
    return 0;
}
