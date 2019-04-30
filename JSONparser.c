#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>

struct Command
{
	int argc;
	char** argv;
};

struct JsonObject
{
	int uid;
	struct Command* command;
};

struct Command* processArguments(int argc, char* argv[]);
void executeCommand(struct Command* command);
void printCommand(struct Command* command);
char* createJsonString(char* json, int uidInt, struct Command* command);
char* createKeyValuePair(char* keyValuePair, char* key, char* value);
char* createJsonCommandArray(char* jsonArray, char* arrayKey, int numValues, char** values);
struct JsonObject* decodeJsonString(char* json, struct JsonObject* jsonObject);
void decodeJsonUid(char* json, struct JsonObject* jsonObject);
void decodeJsonCommand(char* json, struct JsonObject* jsonObject);
void cleanUpJsonObject(struct JsonObject* jsonObject);


/*--------------------------------------------------Command Functions-------------------------------------------------*/
struct Command* processArguments(int argc, char* argv[])
{
	if (argc < 2)	// No Arguments
	{
		char errorMessage[256];
		strcpy(errorMessage, "Usage: ");
		strcat(errorMessage, argv[0]);
		strcat(errorMessage, " cmd cmd - arg1 cmd - arg2...\n");
		fprintf(stderr, errorMessage);
		exit(0);
		//return NULL;
	}
	else			// Create Command
	{
		struct Command* command = (struct Command*)malloc(sizeof(struct Command));
		command->argc = argc - 1;
		if (command->argc != 0)
		{
			command->argv = (char**)calloc((command->argc + 1), sizeof(char*));
			int i, j;
			for (i = 1, j = 0; i < argc; ++i, ++j)
			{
				command->argv[j] = argv[i];
			}
			command->argv[j] = (char*)NULL;
		}
		else
		{
			command->argv = (char**)calloc(2, sizeof(char*));
			command->argv[0] = argv[1];
			command->argv[1] = (char*)NULL;
		}
		return command;
	}
}

void executeCommand(struct Command* command)
{
	char path[256];
	strcpy(path, "/bin/");
	strcat(path, command->argv[0]);
	execv(path, command->argv);
}

// For Testing
void printCommand(struct Command* command)
{
	int i;
	printf("Command Name: %s\n%d Arguments:\n", command->argv[0], command->argc-1);
	for (i = 1; i < command->argc; ++i)
	{
		printf("%s\n", command->argv[i]);
	}
}


/*--------------------------------------------------JSON Functions-------------------------------------------------------*/
/*Encoding*/
char* createJsonString(char* json, int uidInt, struct Command* command)
{
	char functionBuffer[256];
	char uid[256];
	sprintf(uid, "%d", uidInt);

	strcpy(json, "{");
	strcat(json, createKeyValuePair(functionBuffer, "uid", uid));
	strcat(json, ", ");
	strcat(json, createJsonCommandArray(functionBuffer, "command", command->argc, command->argv));
	strcat(json, "}");
	return json;
}

char* createKeyValuePair(char* keyValuePair, char* key, char* value)
{
	strcpy(keyValuePair, "\"");
	strcat(keyValuePair, key);
	strcat(keyValuePair, "\" : \"");
	strcat(keyValuePair, value);
	strcat(keyValuePair, "\"");
	return keyValuePair;
}

char* createJsonCommandArray(char* jsonArray, char* arrayKey, int numValues, char** values)
{
	char argumentNumber[256];
	char functionBuffer[256];
	strcpy(jsonArray, "\"");
	strcat(jsonArray, arrayKey);
	strcat(jsonArray, "\" : {");
	strcat(jsonArray, createKeyValuePair(functionBuffer, "command_name", values[0]));
	
	int i;
	for (i = 1; i < numValues; ++i)
	{
		strcat(jsonArray, ", ");
		strcpy(argumentNumber, "argument_");
		sprintf(functionBuffer, "%d", i);
		strcat(argumentNumber, functionBuffer);
		strcat(jsonArray, createKeyValuePair(functionBuffer, argumentNumber, values[i]));
	}
	strcat(jsonArray, ", ");
	strcat(jsonArray, createKeyValuePair(functionBuffer, "NULL_ptr", "NULL"));
	strcat(jsonArray, "}");
	return jsonArray;
}

/*Decoding*/
struct JsonObject* decodeJsonString(char* json, struct JsonObject* jsonObject)
{
	decodeJsonUid(json, jsonObject);
	decodeJsonCommand(json, jsonObject);
	return jsonObject;
}

void decodeJsonUid(char* json, struct JsonObject* jsonObject)
{
	char functionBuffer[256];
	strncpy(functionBuffer, json + 10, 5);
	jsonObject->uid = strtol(functionBuffer, NULL, 10);
}

void decodeJsonCommand(char* json, struct JsonObject* jsonObject)
{
	/*
	Things to cleanUp
	argv
	arg(s)
	*/

	int argc = 0, commandIndex, i;

	// Find Command Index
	for (i = 0; i < strlen(json); ++i)
	{
		if (0 == strncmp(json + i, "command", 7))
		{
			commandIndex = i;
			break;
		}
	}

	// Count Command Name + Arguments
	commandIndex += 12; // shift over 12 to skip first ':'
	for (i = commandIndex; i < strlen(json); ++i)
	{
		if (json[i] == ':')
		{
			argc++;
		}
	}
	// Remove count for NULL ptr
	argc--;

	// Find the Start and End Indicies of all the strings we need
	int j, quotesToSkip = 2, getStart = 1;

	int* stringStartIndex = (int*)calloc(argc, sizeof(int));
	int* stringEndIndex = (int*)calloc(argc, sizeof(int));
	
	for (i = commandIndex, j = 0; i < strlen(json) && j < argc; ++i)
	{
		if (json[i] == '\"')
		{
			// Get Start Index
			if (quotesToSkip == 0 && getStart)
			{
				stringStartIndex[j] = i + 1;
				getStart = 0;
			}
			// Get End Index
			else if (quotesToSkip == 0 && !getStart)
			{
				stringEndIndex[j++] = i - 1;
				getStart = 1;
				quotesToSkip = 2;
			}
			// Skip
			else
			{
				quotesToSkip--;
			}
		}
	}

	// Use Start and End Indicies to create substring array
	int stringlen;
	char** argv = (char**)calloc((argc + 1), sizeof(char*));

	for (j = 0; j < argc; ++j)
	{
		stringlen = (stringEndIndex[j] - stringStartIndex[j] + 1);
		char* arg = (char*)malloc( (stringlen) * sizeof(char) );
		strncpy(arg, json+stringStartIndex[j], stringlen);
		argv[j] = arg;
	}
	argv[j] = (char*)NULL;

	struct Command* command = (struct Command*)malloc(sizeof(struct Command));
	command->argc = argc;
	command->argv = argv;

	free(stringStartIndex);
	free(stringEndIndex);
	jsonObject->command = command;
}

// Needs work
void cleanUpJsonObject(struct JsonObject* jsonObject)
{
	int i;
	for (i = jsonObject->command->argc; i >= 0; --i)
	{
		free(jsonObject->command->argv[i]);
	}
	free(jsonObject->command->argv);
	free(jsonObject->command);
}


/*--------------------------------------------------Main---------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	char json[4096];
	struct JsonObject jsonObject;
	jsonObject.command = processArguments(argc, argv);
	jsonObject.uid = getuid();

	createJsonString(json, jsonObject.uid, jsonObject.command);
	//cleanUpJsonObject(&jsonObject);
	printf("%s\n", json);
	
	decodeJsonString(json, &jsonObject);
	createJsonString(json, jsonObject.uid, jsonObject.command);
	//cleanUpJsonObject(&jsonObject);
	printf("%s\n", json);

	decodeJsonString(json, &jsonObject);
	createJsonString(json, jsonObject.uid, jsonObject.command);
	printf("%s\n", json);

	executeCommand(jsonObject.command);
}