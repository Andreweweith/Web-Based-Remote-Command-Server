#include "JsonOutput.h"

char *getOutputJsonString(struct output* output_)
{
	// {"stdout":"output_->stdout_", "stderr":"output_->stderr_"}
	char *jsonString = malloc((27 + strlen(output_->stdout_) + strlen(output_->stderr_)) * sizeof(char));
	strcpy(jsonString, "{\"stdout\":\"");
	strcat(jsonString, output_->stdout_);
	strcat(jsonString, "\", \"stderr\":\"");
	strcat(jsonString, output_->stderr_);
	strcat(jsonString, "\"}");
	return jsonString;
}