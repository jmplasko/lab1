#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define BUFFERSIZE 1024
#define LINEMAX 512
#define ARGUMENTSMAX 10
#define PIPELINEMAX 10

void *safe_malloc(size_t size);
void sigintHandler(int sig_num);
int tokenize(char arguments[256][LINEMAX+1], char *input);
int command_checker(char *arguments[], int items);
int execute_commands(char *arguments[], int items);
int prompt_user();
int interpret_script(char script_line[BUFFERSIZE]);
void run_mush();

#endif
