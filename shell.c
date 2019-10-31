#include <stdio.h>
#include <stdlib.h>
#include <signal.h> // This is new!
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <sys/wait.h>

int BUFFER_SIZE = 80;

void help_command(){
        printf("cd: change directory\n");
        printf("help: shows built-in commands\n");
        printf("game: play guessing game\n");
        printf("exit: terminates current shell\n");
}

void cd_command(char* path){
        chdir(path);
}

//guessing game to pick a random number 1-10
void guess_game(){
        int user_guess;
        int num_tries = 0;
        srand(time(NULL));
        printf("Guessing Game: Pick a number 1-10\n");
        int random_num = 1 + (rand() % 10);
        while(user_guess != random_num){
                printf("Make a guess: ");
                scanf("%i", &user_guess);
                num_tries++;
                if(user_guess == random_num){
                        printf("You got it!\nYou took %d guesses\n", num_tries);
                        return;
                }else if(user_guess < random_num){
                        printf("Higher!\n");
                }else{
                        printf("Lower!\n");
                }
        }
}

void system_command(char* line, char* command[], int counter){
        char* bin = "/bin/";
        char str[100];
        strcpy(str, bin);
        strcat(str, line);
        command[0] = str;
        command[counter] = NULL;
        if(execve(command[0], command, NULL) == -1){
                printf("Command not found--Did you mean something else?\n");
        }
}

// Create a signal handler
void sigint_handler(int sig){
        write(1,"\nmini-shell terminated\n",35);
        exit(0);
}

int main(){
        signal(SIGINT, sigint_handler);
        pid_t pid;
        pid_t pid2;
        while(1){
                printf("mini-shell> ");
                int fd[2];
                int ret = pipe(fd);
                bool pipe_present = false;
                char* line = NULL;
                size_t size;
                getline(&line, &size, stdin);
                line[strcspn(line, "\n")] = 0;
                char* user_input = malloc(1000);
                strcpy(user_input, line);
                if(strlen(line) > BUFFER_SIZE){
                        printf("Input too long\n");
                        continue;
                }
		char* token;
                token = strtok(line, " ");
                char* command[16];
                int counter = 0;
                char* command2[16];
                int counter2 = 0;
                while(token != NULL){
                        command[counter] = token;
                        counter++;
                        token = strtok(NULL, " ");
                        if(token != NULL && strcmp(token, "|") == 0){
                                pipe_present = true;
                                break;
                        }
                }
                command[counter] = NULL;
                if(pipe_present == true){
                        int num = 0;
                        token = strtok(user_input, " ");
                        while(token != NULL){
                                if(num > counter){
                                        command2[counter2] = token;
                                        counter2++;
                                }
                                num++;
                                token = strtok(NULL, " ");
                        }
                        command2[counter2] = NULL;
                }
		free(user_input);
                if(pipe_present == true){
                        pid = fork();
                        if(pid == 0){
                                dup2(fd[1], 1);
                                close(fd[0]);
                                execvp(command[0], command);
                        }else{
                                pid2 = fork();
                                if(pid2 == 0){
                                        dup2(fd[0], 0);
                                        close(fd[1]);
                                        execvp(command2[0], command2);
								}else{
                                        close(fd[0]);
                                        close(fd[1]);
                                        waitpid(pid, NULL, 0);
                                        waitpid(pid2, NULL, 0);
                                }
                        }
                        continue;
                }
                if(strcmp(command[0], "exit") == 0){
                        printf("Exiting...\n");
                        break;
                }else if(strcmp(command[0], "help") == 0){
                        help_command();
                }else if(strcmp(command[0], "cd") == 0){
                        cd_command(command[1]);
                }else if(strcmp(command[0], "game") == 0){
                        guess_game();
                        getline(&line, &size, stdin);

                }else{
                        pid = fork();
                        if(pid == 0){
                                system_command(line, command, counter);
                        }else{
                                wait(NULL);
                        }
                }
        }
}

