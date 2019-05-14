#include "JsonOutput.h"

void getOutputJsonString(char* jsonString, struct output* output_)
{
	// {"stdout":"output_->stdout", "stderr":"output_->stderr"}
	strcpy(jsonString, "{\"stdout\":\"");
	strcat(jsonString, output_->stdout);
	strcat(jsonString, "\", \"stderr\":\"");
	strcat(jsonString, output_->stderr);
	strcat(jsonString, "\"}");
}