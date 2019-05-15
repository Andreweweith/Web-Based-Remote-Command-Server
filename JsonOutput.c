#include "JsonOutput.h"

char *getOutputJsonString(struct output* output_)
{
	// {"stdout":"output_->stdout", "stderr":"output_->stderr"}
	char *jsonString = malloc((27 + strlen(output_->stdout) + strlen(output_->stderr)) * sizeof char);
	strcpy(jsonString, "{\"stdout\":\"");
	strcat(jsonString, output_->stdout);
	strcat(jsonString, "\", \"stderr\":\"");
	strcat(jsonString, output_->stderr);
	strcat(jsonString, "\"}");
	return jsonString;
}