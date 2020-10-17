#pragma once

#include <sys/types.h>

#include "parser.h"

void Execute(struct command input);

char** CommandProcessing(struct command *input, char** oSeparation, char** oRedirection);

int  ExePipe(char** process, int pastReadFd, int isHead);

void ExeProcess(char** process, int *pipefds, int infd, char* redirection, int isHead, int isTail);

void ExeChild(char** process, int *pipefds, int infd, char* redirection, int isHead, int isTail);
void ExeParent(char** process, pid_t pid, int *pipefds);

void ExeRedirection(int *pipefds, int fd, char* redirection);

void ExePipeHead(int *pipefds, int infd);
void ExePipeTail(int *pipefds, int infd);
void ExePipeMiddle(int *pipefds, int infd);

