#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "parser.h"

void Execute(struct command input)
{	
	char** arg = (char**)malloc(sizeof(char*) * (input.tokenNumber + 1));
	
	for(int i = 0; i < input.tokenNumber; ++i)
	{
		char* argTemp = input.token[i];
		arg[i] = argTemp;
	}
		
	arg[input.tokenNumber] = NULL; 

	execvp(input.token[0], arg);
}

void ShellMainLoop()
{
	while(1)
	{
		printf("%% ");
		
		char *buffer;
		struct command input;
		size_t index = 0;
		
		getline(&buffer, &index, stdin);
		
		input = ParseCommand(buffer);		
		
		Execute(input);		
	
		if(buffer[0] == '0')
		{
			break;
		}
	}		
}

int main(int argc, char ** argv, char ** envp)
{
	printf("Welcom to Shell\n");

	ShellMainLoop();	

	return 0;
}

