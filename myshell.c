#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80 // maximum command length

void execute_command(char *args[], int background) {
    if (args[0] == NULL) return; // Check if no command

    pid_t pid = fork();

    if (pid < 0) {
        // error in creating process
        fprintf(stderr, "Fork failed!\n");
        exit(1);
    } else if (pid == 0) {
        // child process

        // Handle built-in commands
        if (strcmp(args[0], "cd") == 0) {
            // Change directory
            if (args[1] == NULL) {
                fprintf(stderr, "cd: missing argument\n");
            } else if (chdir(args[1]) != 0) {
                perror("cd error");
                fprintf(stderr, "cd: failed to change directory to '%s'\n", args[1]);
            }
            exit(0);
        } else if (strcmp(args[0], "pwd") == 0) {
            // Print working directory
            char cwd[MAX_LINE];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            } else {
                perror("pwd error");
                fprintf(stderr, "pwd: failed to get current working directory\n");
            }
            exit(0);
        } else if (strcmp(args[0], "clear") == 0) {
            // Clear the terminal screen
            if (system("clear") == -1) {
                perror("clear error");
                fprintf(stderr, "clear: failed to clear the screen\n");
            }
            exit(0);
        }

        // Input/output redirection handling
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], "<") == 0) {
                // input redirection
                int fd = open(args[i + 1], O_RDONLY);
                if (fd < 0) {
                    perror("Input redirection error");
                    fprintf(stderr, "Failed to open input file '%s'\n", args[i + 1]);
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
                args[i] = NULL; // remove from args
            } else if (strcmp(args[i], ">") == 0) {
                // output redirection
                int fd = open(args[i + 1], O_CREAT | O_WRONLY, 0644);
                if (fd < 0) {
                    perror("Output redirection error");
                    fprintf(stderr, "Failed to open output file '%s'\n", args[i + 1]);
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                args[i] = NULL; // remove from args
            }
        }

        // Print the command being executed
        printf("Executing: %s\n", args[0]);

        // Execute the command
        if (execvp(args[0], args) == -1) {
            perror("Execution error");
            fprintf(stderr, "Failed to execute command '%s'\n", args[0]);
        }
        exit(0);
    } else {
        // parent process
        if (background == 0) {
            // wait for child process to complete if not in background
            wait(NULL);
        }
    }
}


void parse_command(char *command, char *args[], int *background) {
    int i = 0;
    char *token;
    // background flag reset
    *background = 0;

    // tokenizing inputs by spaces and handle quotes...new line
    token = strtok(command, "\n");
    // walking through other tokens
    while (token != NULL) {
        // check if token is '&' background processes
        if (strcmp(token, "&") == 0) {
            *background = 1; // mark command as background
        } else if (token[0] == '"') {
            // handled quoted arguments
            char quoted[MAX_LINE] = {0};
            strncpy(quoted, token + 1, strlen(token) - 1); // strip starting quote
            while (token[strlen(token) - 1] != '"' && (token = strtok(NULL, "\n"))) {
                strcat(quoted, " ");
                strcat(quoted, token); // adding remaining quoted string
            }
            quoted[strlen(quoted) - 1] = '\0'; // strip ending quote
            args[i++] = strdup(quoted); // copy the quoted string
        } else {
            args[i++] = token; // adding regular argument
        }
        token = strtok(NULL, "\n");
    }
    // NULL terminate the array of arguments
    args[i] = NULL;
}

int main(void) {
    char *args[MAX_LINE / 2 + 1]; // command line arguments
    char command[MAX_LINE]; // input command
    int background; // background process flag

    while (1) {
        printf("myshell> "); // displaying shell prompt
        fflush(stdout);

        // reading input command
        if (fgets(command, MAX_LINE, stdin) == NULL) {
            break;
        }

        // "exit" will terminate the shell
        if (strcmp(command, "exit\n") == 0) {
            break;
        }

        // parsing command to extract arguments and check for background
        parse_command(command, args, &background);
        // executing command
        execute_command(args, background);
    }
    return 0;
}
