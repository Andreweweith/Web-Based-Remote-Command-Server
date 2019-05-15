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

// Builds linked list based on headerString
struct httpheader* getHttpHeaderStruct(char* headerString)
{
	struct httpheader* httpHeaderPtr = malloc(sizeof(struct httpheader));

	int i, begin, length;
	// Get the First Line
	for (i = 0; i < strlen(headerString); ++i)
	{
		// '\n' functions as a seperator
		if (headerString[i] == '\n')
		{
			httpHeaderPtr->firstline = malloc(i * sizeof(char));
			strncpy(httpHeaderPtr->firstline, headerString, i);
			// Set begin to the first index of the substring that will be passed to the helper method
			begin = i + 1;
			break;
		}
	}

	// Get the Data
	// The data is located at the end of the headerString
	for (i = strlen(headerString) - 1; i >= 0; --i)
	{
		// '\n' functions as a seperator
		if (headerString[i] == '\n')
		{
			++i;
			// strlen(headerString) - i = length of the data
			httpHeaderPtr->data = malloc((strlen(headerString) - i) * sizeof(char));
			// header + i = index of where data starts, strlen(headerString) - i = length of the data
			strncpy(httpHeaderPtr->data, headerString + i, strlen(headerString) - i);
			// Set length to be the length of the substring that will be passed to the helper method
			length = i - begin - 1;
			break;
		}
	}

	// Make a substring with only the remaining header lines
	char* headerSubString = malloc(length * sizeof(char));
	strncpy(headerSubString, headerString + begin, length);

	// Set the next element of the linked list to be the httpheaderitem of the next line
	httpHeaderPtr->item = getHttpItemStruct(headerSubString);
	free(headerSubString);

	return httpHeaderPtr;
}

// Helper method to recursively build linked list
struct httpheaderitem* getHttpItemStruct(char* headerString)
{
	struct httpheaderitem* httpItemPtr = malloc(sizeof(struct httpheaderitem));

	int i, start;

	// Get the Key
	// Example: "Content-Length: 15\n" Key = Content-Length
	for (i = 0; i < strlen(headerString); ++i)
	{
		// ':' functions as a seperator
		if (headerString[i] == ':')
		{
			httpItemPtr->key = malloc(i * sizeof(char));
			strncpy(httpItemPtr->key, headerString, i);
			// Set start to be the start index of the val section of the string
			start = i + 2;
			break;
		}
	}

	// Get the Value
	// Example: "Content-Length: 15\n" Value = 15
	for (i++; i < strlen(headerString); ++i)
	{
		// '\n' functions as a seperator
		if (headerString[i] == '\n')
		{
			// length of val = i - start
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

	// if all lines have been processed and the headerSubString is empty then return the last element of the list
	if (strlen(headerSubString) == 0)
	{
		httpItemPtr->next = NULL;
		free(headerSubString);
		return httpItemPtr;
	}
	// else there are more lines to process
	else
	{
		// set httpItemPtr->next to the result of calling this function again with the reduced headerSubString
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

// Constructs the Http Header String from the linked list
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