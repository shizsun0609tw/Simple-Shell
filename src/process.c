#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "process.h"

void Execute(struct command input)
{	
	DoFork(CommandProcessing(input));
}

char** CommandProcessing(struct command input)
{
	char** arg = (char**)malloc(sizeof(char*) * (input.tokenNumber + 1));
	
	for(int i = 0; i < input.tokenNumber; ++i)
	{
		char* argTemp = input.token[i];
		arg[i] = argTemp;
	}
		
	arg[input.tokenNumber] = NULL; 

	return arg;
}

void DoFork(char** arg)
{
	pid_t PID = fork();

	switch(PID)
	{
		case -1:
			break;
		case 0:
			ExeChild(arg);
			break;
		default:
			ExeParent(arg, PID);
			break;
	}
}

void ExeChild(char** arg)
{
	execvp(arg[0], arg);
}

void ExeParent(char** arg, pid_t PID)
{
	pid_t waitPid;
	int status;
	
	while(1)
	{
		waitPid = waitpid(PID, &status, WNOHANG);
	
		if (waitPid == PID) break;
	}
}
