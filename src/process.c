#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "process.h"

void Execute(struct command input)
{	
	char* separation = (char*)malloc(sizeof(char));
	char* redirection = NULL;
	char** process1 = CommandProcessing(&input, &separation, &redirection);
	int pastReadFd = 0;
	int isHead = 1;

	while(input.currentCommandNumber != input.tokenNumber)
	{
		char** process2 = CommandProcessing(&input, &separation, &redirection);	
		
		if (strcmp(separation, "|") == 0) 
		{
			pastReadFd = ExeProcessPipe(process1, pastReadFd, isHead);
		}

		isHead = 0;
		free(process1);
		process1 = process2;
	}
		
	ExeProcess(process1, NULL, pastReadFd, redirection, isHead, 1);
	
	if (pastReadFd != 0) close(pastReadFd);

	free(input.token);
	free(process1);
}

char** CommandProcessing(struct command *input, char** oSeparation, char** oRedirection)
{
	char** process = (char**)malloc(sizeof(char*) * (input->tokenNumber + 1));

	for(int count = 0; input->currentCommandNumber < input->tokenNumber; ++count, ++input->currentCommandNumber)
	{
		char* argTemp = input->token[input->currentCommandNumber];
		
		if (strcmp(argTemp, "|") == 0)
		{
			*oSeparation = "|";
			process[count] = NULL;
			++input->currentCommandNumber;		
	
			break;
		}
		else if(strcmp(argTemp, ">") == 0)
		{
			++input->currentCommandNumber;
			*oRedirection = input->token[input->currentCommandNumber];
		}
		else
		{	
			process[count] = argTemp;
			process[count + 1] = NULL;
		}
	}

	return process;
}

int ExeProcessPipe(char** process, int pastReadFd, int isHead)
{
	int* pipefds = (int*)malloc(sizeof(int) * 2);
	int readFd;

	if (pipe(pipefds) == -1)
	{
		printf("pipe error\n");
	}
	
	ExeProcess(process, pipefds, pastReadFd, NULL, isHead, 0);

	readFd = pipefds[0];

	if (pastReadFd != 0) close(pastReadFd);

	free(pipefds);

	return readFd;	
}

void ExeProcess(char** process, int *pipefds, int infd, char* redirection, int isHead, int isTail)
{
	if (strcmp(process[0], "exit") == 0)
	{
		ExeExit();
	}
	else if (strcmp(process[0], "printenv") == 0)
	{
		ExePrintEnv(process);
		return;
	}
	else if (strcmp(process[0], "setenv") == 0)
	{
		ExeSetEnv(process);
		return;
	}

	pid_t pid = fork();
	
	switch(pid)
	{
		case -1:
			printf("fork error\n");
			break;
		case 0:
			ExeChild(process, pipefds, infd, redirection, isHead, isTail);
			break;
		default:
			ExeParent(process, pid, pipefds);
			break;
	}
}

void ExeExit(char** process)
{
	exit(EXIT_SUCCESS);
}

void ExeSetEnv(char** process)
{
	if (process[1] == NULL || process[2] == NULL) return;

	if (setenv(process[1], process[2], 1) == -1)
	{
		printf("setenv error\n");
	}	
}

void ExePrintEnv(char** process)
{
	char* env;
	
	if (process[1] != NULL)
	{
		env = getenv(process[1]);
		
		printf("%s\n", (env == NULL ? "parameter not found" : env));	
	}
}

void ExeChild(char** process, int *pipefds, int infd, char* redirection, int isHead, int isTail)
{
	int isPipe = (isHead && isTail ? 0 : 1);
	int isRedirection = (redirection != NULL ? 1 : 0);

	if (isRedirection == 1) ExeRedirection(pipefds, infd, redirection);

	if (isPipe == 0 || isRedirection == 1) execvp(process[0], process);

	if (isPipe == 1) ExePipe(process, pipefds, infd, isHead, isTail);
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

void ExeRedirection(int *pipefds, int infd, char* redirection)
{
	int fd = open(redirection, O_APPEND | O_CREAT | O_WRONLY, 0644); 
		
	dup2(fd, STDOUT_FILENO);
	dup2(infd, STDIN_FILENO);

	if (pipefds != NULL)
	{
		close(pipefds[0]);
		close(pipefds[1]);
	}

	close(infd);
	close(fd);
}

void ExePipe(char** process, int *pipefds, int infd, int isHead, int isTail)
{
	if (isHead)
	{
		ExePipeHead(pipefds, infd);
	}
	else if (isTail)
	{
		ExePipeTail(pipefds, infd);
	}
	else
	{
		ExePipeMiddle(pipefds, infd);
	}
	
	execvp(process[0], process);
}

void ExePipeHead(int *pipefds, int infd)
{
	close(pipefds[0]);
	dup2(pipefds[1], STDOUT_FILENO);
	close(pipefds[1]);
}

void ExePipeTail(int *pipefds, int infd)
{
	if (pipefds != NULL)
	{
		close(pipefds[0]);
		close(pipefds[1]);
	}

	dup2(infd, STDIN_FILENO);
	close(infd);
}

void ExePipeMiddle(int *pipefds, int infd)
{
	close(pipefds[0]);
	dup2(infd, STDIN_FILENO);
	dup2(pipefds[1], STDOUT_FILENO);
	close(infd);
	close(pipefds[1]);
}
