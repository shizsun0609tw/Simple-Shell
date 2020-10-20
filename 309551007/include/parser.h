#pragma once

#include <stdio.h>

struct command{
	size_t tokenNumber;
	size_t maxSize;
	size_t currentCommandNumber;
	char** token;
};

struct command ParseCommand(char* buffer);

int IsNumberPipe(char* buffer, int *oNumber);
