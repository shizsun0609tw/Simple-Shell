#pragma once

#include <stdio.h>

struct command{
	size_t tokenNumber;
	size_t maxSize;
	size_t currentCommandNumber;
	char** token;
};

struct enviromentParameter{
	size_t number;
	size_t maxSize;
	char** parameter;
};

struct command ParseCommand(char* buffer);
struct command ParseEnviromentParameter(char* buffer);

