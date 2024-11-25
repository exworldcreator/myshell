#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_ARGS 10
#define MAX_HISTORY 10

void load_command_history() {
    FILE *file = fopen("commands.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }    

    char history_buffer[100];
    printf("Command History:\n");
    while(fgets(history_buffer, sizeof(history_buffer), file) != NULL) {
        printf("%s", history_buffer);
    }

    fclose(file);
}

void command_history_saver(char *buffer) {
    if (strlen(buffer) > 0) {
        FILE *file = fopen("commands.txt", "a");
        if (file == NULL) {
            perror("Error opening file for logging command");
            return;
    }
        fprintf(file, "%s\n", buffer);
        fclose(file);
    }
}

void redirection_handler(char *buffer, char *args[]) {
    char *input_redir_pos = strchr(buffer, '<');
    char *output_redir_pos = strchr(buffer, '>');

    if (output_redir_pos != NULL) {
        char *file_name = strtok(output_redir_pos + 1, " ");

        if (file_name != NULL) {
            FILE *output_file = fopen(file_name, "w");
            if (output_file == NULL) {
                perror("Error opening file");
                return;
            }

            if (freopen(file_name, "w", stdout) == NULL) {
                perror("Error redirecting stdout to file");
                return;
            }
        }
    }

    if (input_redir_pos != NULL) {
        char *file_name = strtok(input_redir_pos + 1, " ");

        if (file_name != NULL) {
            FILE *input_file = fopen(file_name, "r");
            if (input_file == NULL) {
                perror("Error openning file for input redirection");
                return;
            }

            if (freopen(file_name, "r", stdin) == NULL) {
                perror("Error redirecting stdin from file");
                return;
            }
        }
    }
}

void background_process_handler(char *args[]) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("execvp");
        }
        exit(1);
    } else {
        printf("Process running in background with PID: %d\n", pid);
    }
}

void arguments_handler(char *buffer, char *args[]) {
    char *token;
    int i = 0;

    token = strtok(buffer, " ");
    while (token!= NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

void handler_cd(char *buffer) {
    char *dir = buffer + 3;
    if (chdir(dir) != 0) {
        perror("cd");
    }
}

void execute_command(char *args[]) {
    if (execvp(args[0], args) == -1) {
        perror("execvp");
    }
}

int main() {
    char buffer[100];
    char *args[MAX_ARGS];
    char directory[100] = "mysh> / ";

    while (1) {
        printf("%s", directory);
        fgets(buffer, sizeof(buffer), stdin);

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "exit") == 0) {
            break;
        } else if (strcmp(buffer, "history") == 0) {
            load_command_history();
            continue;
        }
 
        int background = 0;
        if (buffer[strlen(buffer) - 1] == '&') {
            background = 1;
            buffer[strlen(buffer) - 1] = '\0';
        }

        if (strncmp(buffer, "cd", 2) == 0) {
            handler_cd(buffer);
            char cwd[100];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                snprintf(directory, sizeof(directory), "mysh> %s ", cwd);
            } else {
                perror("getcwd");
            }
            continue;
        }

        arguments_handler(buffer, args);
        
        redirection_handler(buffer, args);

        command_history_saver(buffer);

        if (background) {
            background_process_handler(args);
        } else {
            execute_command(args);
        }
    }

    return 0;
}
