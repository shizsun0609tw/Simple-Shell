#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include "process.h"

void Execute(struct command input)
{	
	char** process1 = CommandProcessing(&input);
	int* pipefds;
	int pastReadFd = 0;
	int isHead = 1;

	while(input.currentCommandNumber != input.tokenNumber)
	{
		char** process2 = CommandProcessing(&input);
		pipefds = ExePipe();
		
		ExeProcess(process1, pipefds, pastReadFd, isHead, 0);

		isHead = 0;
		free(process1);	
		process1 = process2;
		pastReadFd = pipefds[0];
		if(input.currentCommandNumber != input.tokenNumber) free(pipefds);
	}
		
	ExeProcess(process1, pipefds, pastReadFd, isHead, 1);
	free(pipefds);
}

char** CommandProcessing(struct command *input)
{
	char** process = (char**)malloc(sizeof(char*) * (input->tokenNumber + 1));

	for(int currentNumber = input->currentCommandNumber, count = 0; currentNumber < input->tokenNumber; ++currentNumber, ++count)
	{
		char* argTemp = input->token[currentNumber];
		
		input->currentCommandNumber++;
		
		if (strcmp(argTemp, "|") == 0)
		{
			process[count] = NULL;
			break;
		}
		else
		{	
			process[count] = argTemp;
			process[count + 1] = NULL;
		}
	}

	return process;
}

int* ExePipe()
{
	int *pipefds = (int*)malloc(sizeof(int) * 2);

	if (pipe(pipefds) == -1)
	{
		printf("pipe error\n");
	}
	
	return pipefds;	
}

void ExeProcess(char** process, int *pipefds, int infd, int isHead, int isTail)
{
	pid_t pid = fork();
	
	switch(pid)
	{
		case -1:
			printf("fork error\n");
			break;
		case 0:
			ExeChild(process, pipefds, infd, isHead, isTail);
			break;
		default:
			ExeParent(process, pid, pipefds);
			break;
	}
}

void ExeChild(char** process, int *pipefds, int infd, int isHead, int isTail)
{
	if (pipefds != NULL)
	{
		if (isHead)
		{
			close(pipefds[0]);
			dup2(pipefds[1], STDOUT_FILENO);
			close(pipefds[1]);
		}
		else if (isTail)
		{
			close(pipefds[1]);
			dup2(pipefds[0], STDIN_FILENO);
			close(pipefds[0]);			
		}
		else
		{
			close(pipefds[0]);
			dup2(infd, STDIN_FILENO);
			dup2(pipefds[1], STDOUT_FILENO);
			close(infd);
			close(pipefds[1]);
		}
	}
	
	execvp(process[0], process);
}

void ExeParent(char** process, pid_t pid, int *pipefds)
{
	pid_t waitPid;
	int status;
	
	while(1)
	{
		waitPid = waitpid(pid, &status, WNOHANG);
		
		if (pipefds != NULL)
		{	
			close(pipefds[1]);
		}
	
		if (waitPid == pid) break;
	}
}
