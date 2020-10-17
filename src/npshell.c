#include <stdio.h>

#include "parser.h"
#include "process.h"

void InitPath()
{
	setenv("PATH", "/bin/", 1);	
}

void ShellMainLoop()
{
	while(1)
	{
		printf("%% ");
		
		char *buffer;
		struct command input;
		size_t index = 0;
		
		if (getline(&buffer, &index, stdin) == EOF)
		{
			printf("\n");
			break;
		}
		
		input = ParseCommand(buffer);		
		
		Execute(input);	
	}		
}

int main(int argc, char **argv, char **envp)
{
	InitPath();	

	printf("Welcom to Shell\n");

	ShellMainLoop();	

	return 0;
}

