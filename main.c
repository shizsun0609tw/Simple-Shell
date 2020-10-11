#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

struct command{
	size_t tokenNumber;
	size_t maxSize;
	char** token;
};

struct command Parser(char* buffer)
{
	struct command input;
	
	char* digit = " \n";
	char* tempToken;

	input.tokenNumber = 0;
	input.maxSize = 1;

	tempToken = strtok(buffer, digit);
	input.token = (char**)malloc(sizeof(char*));	


	while(tempToken != NULL)
	{
		input.token[input.tokenNumber] = (char*)malloc(sizeof(tempToken));
		input.token[input.tokenNumber] = tempToken;

		input.tokenNumber++;
		
		if(input.tokenNumber >= input.maxSize / 2)
		{
			input.maxSize *= 2;
			input.token = (char**)realloc(input.token, sizeof(char*) * input.maxSize);
		}
	
		tempToken = strtok(NULL, digit);			
	}
	
	return input;
}

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
		
		input = Parser(buffer);		
		
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

