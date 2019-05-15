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

	int i, begin, length;
	// Get the First Line
	for (i = 0; i < strlen(headerString); ++i)
	{
		if (headerString[i] == '\n')
		{
			httpHeaderPtr->firstline = malloc(i * sizeof(char));
			strncpy(httpHeaderPtr->firstline, headerString, i);
			begin = i + 1;
			break;
		}
	}

	// Get the Data
	for (i = strlen(headerString) - 1; i >= 0; --i)
	{
		if (headerString[i] == '\n')
		{
			++i;
			// strlen(headerString) - i = length of the data
			httpHeaderPtr->data = malloc((strlen(headerString) - i) * sizeof(char));
			// header + i = address of where data starts, strlen(headerString) - i = length of the data
			strncpy(httpHeaderPtr->data, headerString + i, strlen(headerString) - i);
			length = i - begin - 1;
			break;
		}
	}

	// Make a substring with only the remaining header lines
	//char* headerSubString = malloc(length * sizeof(char));
	//strncpy(headerSubString, headerString + begin, length);

	//httpHeaderPtr->item = getHttpItemStruct(headerSubString);
	//free(headerSubString);

	return httpHeaderPtr;
}

struct httpheaderitem* getHttpItemStruct(char* headerString)
{
	struct httpheaderitem* httpItemPtr = malloc(sizeof(struct httpheaderitem));

	int i, start;

	// Get the Key
	for (i = 0; i < strlen(headerString); ++i)
	{
		if (headerString[i] == ':')
		{
			httpItemPtr->key = malloc(i * sizeof(char));
			strncpy(httpItemPtr->key, headerString, i);
			start = i + 2;
			break;
		}
	}

	// Get the Value
	for (i++; i < strlen(headerString); ++i)
	{
		if (headerString[i] == '\n')
		{
			// length of val = i -start
			httpItemPtr->val = malloc((i - start) * sizeof(char));
			// headerString + start = address of where val starts, i - start = length of the val
			strncpy(httpItemPtr->val, headerString + start, (i - start));
			break;
		}
	}

	// Make a substring with only the remaining header lines
	int length = strlen(headerString) - i;
	char* headerSubString = malloc(length * sizeof(char));
	strncpy(headerSubString, headerString + i + 1, length);

	if (strlen(headerSubString) == 0)
	{
		httpItemPtr->next = NULL;
		free(headerSubString);
		return httpItemPtr;
	}
	else
	{
		httpItemPtr->next = getHttpItemStruct(headerSubString);
		free(headerSubString);
		return httpItemPtr;
	}
}

/*
"HTTP/1.1 200 OK\n"
"Content-Type: text/html\n"
"Content-Length: 15\n"
"Accept-Ranges: bytes\n"
"Connection: keep-alive\n"
"\n"
"sdfkjsdnbfkjbsf"
*/

void getHttpHeaderString(char* headerString, struct httpheader* httpHeaderPtr)
{
	// Add first line
	strcpy(headerString, httpHeaderPtr->firstline);
	strcat(headerString,"\n");
	// Loop through list and add lines
	struct httpheaderitem* item = httpHeaderPtr->item;
	while (item != NULL)
	{
		strcat(headerString, item->key);
		strcat(headerString, ": ");
		strcat(headerString, item->val);
		strcat(headerString, "\n");
		item = item->next;
	}
	// Add data
	strcat(headerString, "\n");
	strcat(headerString, httpHeaderPtr->data);
	strcat(headerString, "\0");
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