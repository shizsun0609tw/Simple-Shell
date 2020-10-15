#pragma once

#include <sys/types.h>

#include "parser.h"

void Execute(struct command input);

char** CommandProcessing(struct command *input, char** oSeparation);

void ExeProcess(char** process, int *pipefds, int infd, int isHead, int isTail);
int* ExePipe();
void ExeChild(char** process, int *pipefds, int infd, int isHead, int isTail);
void ExeParent(char** process, pid_t pid, int *pipefds);
