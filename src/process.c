#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "process.h"

void InitPipeTable(struct pipeTable *numberPipeTable, const int tableSize)
{
	numberPipeTable->tableSize = tableSize;
	numberPipeTable->lineCountTable = (int**)malloc(sizeof(int*) * tableSize);
	
	for(int i = 0; i < tableSize; ++i)
	{
		numberPipeTable->lineCountTable[i] = (int*)malloc(sizeof(int) * 2);
		numberPipeTable->lineCountTable[i][0] = 0;
		numberPipeTable->lineCountTable[i][1] = 0;
	}
}

void Execute(struct command input)
{
	static int IsAlreadyInit = 0;
	static struct pipeTable numberPipeTable;
	char** process1 = NULL;
	char* redirection = NULL;
	char* separation = (char*)malloc(sizeof(char));
	int numberPipe = 0;
	int numberPipefd = 0;
	int pastReadFd = 0;
	int isHead = 1;

	if (IsAlreadyInit == 0)
	{
		InitPipeTable(&numberPipeTable, 1001);
		IsAlreadyInit = 1;
	}

	process1 = CommandProcessing(&input, &separation, &redirection, &numberPipe);

	UpdateNumberPipe(&numberPipeTable, &numberPipefd);
	
	while(input.currentCommandNumber != input.tokenNumber)
	{
		char** process2 = CommandProcessing(&input, &separation, &redirection, &numberPipe);	
		
		if (strcmp(separation, "|") == 0) 
		{
			pastReadFd = ExeProcessPipe(process1, pastReadFd, NULL, numberPipefd, isHead);
			numberPipefd = 0;
		}

		isHead = 0;
		free(process1);
		process1 = process2;
	}
		
	if (numberPipe > 0)
	{	
		ExeProcessNumberPipe(process1, pastReadFd, &numberPipeTable, numberPipefd, separation, numberPipe, isHead);
	}
	else 
	{
		ExeProcess(process1, NULL, pastReadFd, NULL, numberPipefd, redirection, isHead, 1);
	}
	
	if (pastReadFd != 0) close(pastReadFd);

	free(input.token);
	free(process1);
}

char** CommandProcessing(struct command *input, char** oSeparation, char** oRedirection, int *oNumberPipe)
{
	char** process = (char**)malloc(sizeof(char*) * (input->tokenNumber + 1));

	for(int count = 0; input->currentCommandNumber < input->tokenNumber; ++count, ++input->currentCommandNumber)
	{
		char* argTemp = input->token[input->currentCommandNumber];
		
		if (strcmp(argTemp, "|") == 0 || IsNumberPipe(argTemp, oNumberPipe))
		{
			if (argTemp[0] == '!') *oSeparation = "!";
			else *oSeparation = "|";
			
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

void UpdateNumberPipe(struct pipeTable *numberPipeTable, int* ofd)
{
	for(int i = 1; i < numberPipeTable->tableSize; ++i)
	{	
		numberPipeTable->lineCountTable[i - 1][0] = numberPipeTable->lineCountTable[i][0];
		numberPipeTable->lineCountTable[i - 1][1] = numberPipeTable->lineCountTable[i][1];
		numberPipeTable->lineCountTable[i][0] = 0;
		numberPipeTable->lineCountTable[i][1] = 0;
	}

	if (numberPipeTable->lineCountTable[0][0] != 0) 
	{
		*ofd = numberPipeTable->lineCountTable[0][0];
		numberPipeTable->lineCountTable[0][0] = 0;	
		close(numberPipeTable->lineCountTable[0][1]);
	}
}

int ExeProcessPipe(char** process, int pastReadFd, char* numberPipeSeparation, int numberPipefd, int isHead)
{
	int* pipefds = (int*)malloc(sizeof(int) * 2);
	int readFd;
	

	if (pipe(pipefds) == -1)
	{
		printf("pipe error\n");
	}
 
	ExeProcess(process, pipefds, pastReadFd, numberPipeSeparation, numberPipefd, NULL, isHead, 0);

	readFd = pipefds[0];

	if (pastReadFd != 0) close(pastReadFd);

	free(pipefds);

	return readFd;	
}

void ExeProcessNumberPipe(char** process, int pastReadFd, struct pipeTable *numberPipeTable, int numberPipefd, char* separation, int line, int isHead)
{
	int* pipefds = (int*)malloc(sizeof(int) * 2);

	if (numberPipeTable->lineCountTable[line][0] == 0)
	{
		pipe(pipefds);
		
		numberPipeTable->lineCountTable[line][0] = pipefds[0];
		numberPipeTable->lineCountTable[line][1] = pipefds[1];
	}
	else
	{
		pipefds[0] = numberPipeTable->lineCountTable[line][0];
		pipefds[1] = numberPipeTable->lineCountTable[line][1];
	}

	ExeProcess(process, pipefds, pastReadFd, separation, numberPipefd, NULL, isHead, 0);
	
	free(pipefds);
}

void ExeSource(char** process)
{
	FILE *fp = fopen(process[1], "r");
	char* buffer = NULL;
	size_t buffer_size = 0;
	int line_size = 0;

	line_size = getline(&buffer, &buffer_size, fp);

	while(line_size > 0)
	{
		Execute(ParseCommand(buffer));
		line_size = getline(&buffer, &buffer_size, fp);
	}	
}

void ExeProcess(char** process, int *pipefds, int infd, char* numberPipeSeparation, int numberPipefd, char* redirection, int isHead, int isTail)
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
	else if (strcmp(process[0], "source") == 0)
	{
		ExeSource(process);
		return;
	}
	
	pid_t pid = fork();
	
	if (pid == -1) 
	{
		ExeWait(pid);
		pid = fork();
	}

	switch(pid)
	{
		case -1:
			
			printf("fork error\n");
			break;
		case 0:
			ExeChild(process, pipefds, infd, numberPipeSeparation, numberPipefd, redirection, isHead, isTail);
			break;
		default:
			ExeParent(process, pid, pipefds, infd, (numberPipeSeparation != NULL), numberPipefd, isTail);
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

	setenv(process[1], process[2], 1);	
}

void ExePrintEnv(char** process)
{
	char* env;
	
	if (process[1] != NULL)
	{
		env = getenv(process[1]);
		if (env != NULL) printf("%s\n", env);
	}
}

void ExeChild(char** process, int *pipefds, int infd, char* numberPipeSeparation, int numberPipefd, char* redirection, int isHead, int isTail)
{
	int isPipe = (isHead && isTail ? 0 : 1);
	int isRedirection = (redirection != NULL ? 1 : 0);
		
	if (numberPipefd > 0) ExeNumberPipe(numberPipefd);
	
	if (isRedirection == 1) ExeRedirection(pipefds, infd, redirection);

	if (isPipe == 0 && numberPipefd > 0) close(numberPipefd);
	
	if (isPipe == 0 || isRedirection == 1) DoExecvp(process[0], process);
	
	if (isPipe == 1) ExePipe(process, pipefds, infd, numberPipeSeparation, isHead, isTail);
}

void ExeParent(char** process, pid_t pid, int *pipefds, int infd, int isNumberPipe, int numberPipefd, int isTail)
{
	if (numberPipefd > 0) close(numberPipefd);
	if (pipefds != NULL && isNumberPipe == 0) close(pipefds[1]);
	if (infd > 0) close(infd);

	if (isTail == 0) return;
	else ExeWait(pid);
}

void ExeRedirection(int *pipefds, int infd, char* redirection)
{
	int fd = open(redirection, O_TRUNC | O_CREAT | O_WRONLY, 0644); 

	dup2(fd, STDOUT_FILENO);
	
	if (infd > 0) 
	{
		dup2(infd, STDIN_FILENO);
		close(infd);
	}

	if (pipefds != NULL)
	{
		close(pipefds[0]);
		close(pipefds[1]);
	}

	close(fd);
}

void ExePipe(char** process, int *pipefds, int infd, char* numberPipeSeparation, int isHead, int isTail)
{	
	if (isHead)
	{
		ExePipeHead(pipefds, numberPipeSeparation, infd);
	}
	else if (isTail)
	{	
		ExePipeTail(pipefds, infd);
	}
	else
	{
		ExePipeMiddle(pipefds, numberPipeSeparation, infd);
	}
	
	DoExecvp(process[0], process);
}

void ExeNumberPipe(int numberPipefd)
{
	dup2(numberPipefd, STDIN_FILENO);
	close(numberPipefd);
}

void ExePipeHead(int *pipefds, char* numberPipeSeparation, int infd)
{	
	close(pipefds[0]);
	dup2(pipefds[1], STDOUT_FILENO);
	
	if (numberPipeSeparation != NULL && numberPipeSeparation[0] == '!') 
	{
		dup2(pipefds[1], STDERR_FILENO);
	}
	close(pipefds[1]);
}

void ExePipeTail(int *pipefds, int infd)
{
	if (pipefds != NULL)
	{
		if (pipefds[0] != 0) close(pipefds[0]);
		if (pipefds[1] != 0) close(pipefds[1]);
	}

	dup2(infd, STDIN_FILENO);
	close(infd);
}

void ExePipeMiddle(int *pipefds, char* numberPipeSeparation, int infd)
{
	if (pipefds[0] != 0) close(pipefds[0]);
	dup2(infd, STDIN_FILENO);
	dup2(pipefds[1], STDOUT_FILENO);
	if (numberPipeSeparation != NULL && numberPipeSeparation[0] == '!')
	{
		dup2(pipefds[1], STDERR_FILENO);
	}
	close(infd);
	close(pipefds[1]);
}

void ExeWait(pid_t pid)
{
	int status;
	int waitPID;
	
	while(1)
	{
		waitPID = waitpid(pid, &status, WNOHANG);

		if (waitPID == pid) break;
	}
}

void DoExecvp(char* process, char** arg)
{
	if(execvp(process, arg) == -1)
	{
		fprintf(stderr, "Unknown command: [%s].\n", process);
		exit(EXIT_FAILURE);
	}
}
