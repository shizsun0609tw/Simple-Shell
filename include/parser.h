#include <stdio.h>

struct command{
	size_t tokenNumber;
	size_t maxSize;
	char** token;
};

struct command ParserCommand(char* buffer);
