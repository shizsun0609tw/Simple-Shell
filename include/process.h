#pragma once

#include <sys/types.h>

#include "parser.h"

void Execute(struct command input);

char** CommandProcessing(struct command input);

void DoFork(char** arg);

void ExeChild(char** arg);
void ExeParent(char** arg, pid_t PID);
