#include "HttpHeaderInterpreter.h"

/*
"HTTP/1.1 200 OK\n"
"Content-Type: text/html\n"
"Content-Length: 15\n"
"Accept-Ranges: bytes\n"
"Connection: keep-alive\n"
"\n"
"sdfkjsdnbfkjbsf"
*/

struct httpheader* getHttpHeaderStruct(char* headerString)
{
	struct httpheader* httpHeaderPtr = malloc(sizeof(struct httpheader));
	// Create a copy of headerString to tokenize
	char* tempHeaderString = malloc((strlen(headerString) + 1) * sizeof(char));
	strcpy(tempHeaderString, headerString);
	// Used to Free the allocated memory of tempHeaderString
	char* freeTempPtr = tempHeaderString;
	// Used in tokenizer
	char* token;

	// Find the number of lines so it is known when the tokenizer has reached the data
	int i, numLines;
	for (i = 0, numLines = 0; i < strlen(tempHeaderString); ++i)
	{
		if (tempHeaderString[i] == '\n')
		{
			numLines++;
		}
	}
	// Tokenize all of the lines
	i = 0;
	while ((token = strtok_r(tempHeaderString, "\n", &tempHeaderString)))
	{
		// Get the First Line
		if (i == 0)
		{
			httpHeaderPtr->firstline = malloc(strlen(token) * sizeof(char));
			strcpy(httpHeaderPtr->firstline, token);
		}
		// Get the Data Line
		else if (i == numLines - 1)
		{
			httpHeaderPtr->data = malloc(strlen(token) * sizeof(char));
			strcpy(httpHeaderPtr->data, token);
		}
		++i;
	}

	httpHeaderPtr->item = NULL;
	free(freeTempPtr);

	return httpHeaderPtr;
}

void freeHeader(struct httpheader* httpHeaderPtr)
{
	// Free the httpheader struct
	free(httpHeaderPtr->firstline);
	free(httpHeaderPtr->data);
	struct httpheaderitem* item = httpHeaderPtr->item;
	free(httpHeaderPtr);
	// Free all of the httpheaderitem structs
	struct httpheaderitem* iter = item;
	while (iter != NULL)
	{
		item = iter;
		free(item->key);
		free(item->val);
		iter = item->next;
		free(item);
	}
}