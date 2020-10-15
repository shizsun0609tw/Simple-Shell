#include <stdlib.h>
#include <string.h>

#include "parser.h"

struct command ParseCommand(char* buffer)
{
	struct command input;

	char* digit = " \n";
	char* tempToken;

	input.tokenNumber = 0;
	input.maxSize = 1;
	input.currentCommandNumber = 0;

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