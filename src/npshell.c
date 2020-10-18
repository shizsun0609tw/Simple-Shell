#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "process.h"

void InitPath()
{
	setenv("PATH", "bin:.", 1);	
}

void ShellMainLoop()
{
	while(1)
	{
		printf("%% ");
		
		char *buffer = NULL;
		struct command input;
		size_t index = 0;
						
		if (getline(&buffer, &index, stdin) == EOF)
		{
			printf("\n");
			break;
		}
				
		input = ParseCommand(buffer);		

		if(input.tokenNumber != 0) Execute(input);		
		
		free(buffer);
	}		
}

int main(int argc, char **argv, char **envp)
{
	InitPath();	

	ShellMainLoop();	

	return 0;
}

