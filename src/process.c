#include <stdlib.h>
#include <unistd.h>

#include "process.h"

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



