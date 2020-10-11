#include <stdio.h>
#include <stdlib.h>

void ShellMainLoop()
{
	while(1)
	{
		printf("%% ");
		
		char *buffer;
		size_t index = 0;
		
		getline(&buffer, &index, stdin);
	
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

