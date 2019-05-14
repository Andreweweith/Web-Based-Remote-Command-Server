#ifndef HTTP_INTERPRETER_H
#define HTTP_INTERPRETER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"

struct httpheader* getHttpHeaderStruct(char* headerString);

struct httpheaderitem* getHttpItemStruct(char* headerString);

void getHttpHeaderString(char* headerString, struct httpheader* httpHeaderPtr);

void freeHeader(struct httpheader* httpHeaderPtr);

#endif