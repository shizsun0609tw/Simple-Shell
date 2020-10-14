#include <stdio.h>

#include "parser.h"
#include "process.h"

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
	}		
}

int main(int argc, char ** argv, char ** envp)
{
	printf("Welcom to Shell\n");

	ShellMainLoop();	

	return 0;
}

