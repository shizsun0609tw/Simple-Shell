#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include "process.h"

void Execute(struct command input)
{	
	char* separation = (char*)malloc(sizeof(char));
	char** process1 = CommandProcessing(&input, &separation);
	int pastReadFd = 0;
	int isHead = 1;

	while(input.currentCommandNumber != input.tokenNumber)
	{
		char** process2 = CommandProcessing(&input, &separation);	
		
		if (strcmp(separation, "|") == 0) 
		{
			pastReadFd = ExePipe(process1, pastReadFd, isHead);
		}
		else if (strcmp(separation, ">") == 0)
		{
			printf("redirection\n");
		}

		isHead = 0;
		free(process1);
		process1 = process2;
	}
	
	ExeProcess(process1, NULL, pastReadFd, isHead, 1);
	
	if (pastReadFd != 0) close(pastReadFd);

	free(input.token);
	free(process1);
}

char** CommandProcessing(struct command *input, char** oSeparation)
{
	char** process = (char**)malloc(sizeof(char*) * (input->tokenNumber + 1));

	for(int currentNumber = input->currentCommandNumber, count = 0; currentNumber < input->tokenNumber; ++currentNumber, ++count)
	{
		char* argTemp = input->token[currentNumber];
		
		input->currentCommandNumber++;
		
		if (strcmp(argTemp, "|") == 0)
		{
			*oSeparation = "|";
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

int ExePipe(char** process, int pastReadFd, int isHead)
{
	int* pipefds = (int*)malloc(sizeof(int) * 2);
	int readFd;

	if (pipe(pipefds) == -1)
	{
		printf("pipe error\n");
	}
	
	ExeProcess(process, pipefds, pastReadFd, isHead, 0);

	readFd = pipefds[0];

	if (pastReadFd != 0) close(pastReadFd);

	free(pipefds);

	return readFd;	
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
	if (isHead && isTail) execvp(process[0], process);

	if (isHead)
	{	
		close(pipefds[0]);
		dup2(pipefds[1], STDOUT_FILENO);
		close(pipefds[1]);
	}
	else if (isTail)
	{
		if (pipefds != NULL)
		{
			close(pipefds[0]);
			close(pipefds[1]);
		}

		dup2(infd, STDIN_FILENO);
		close(infd);	
	}
	else
	{
		close(pipefds[0]);
		dup2(infd, STDIN_FILENO);
		dup2(pipefds[1], STDOUT_FILENO);
		close(infd);
		close(pipefds[1]);
	}
	
	execvp(process[0], process);
}

void ExeParent(char** process, pid_t pid, int *pipefds)
{
	pid_t waitPid;
	int status;

	if (pipefds != NULL) close(pipefds[1]);
	
	while(1)
	{
		waitPid = waitpid(pid, &status, WNOHANG);
		
		if (waitPid == pid) break;
	}
}
