#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "URIDecoder.h"
#include "structs.h"

// Credit to https://stackoverflow.com/a/14530993
// Replaces %## with appropriate special character
void remove_special_characters(char *dst, const char *src) {
	char a, b;
	while (*src) {
		if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b))) {
			if (a >= 'a')
				a -= 'a' - 'A';
			if (a >= 'A')
				a -= ('A' - 10);
			else
				a -= '0';
			if (b >= 'a')
				b -= 'a' - 'A';
			if (b >= 'A')
				b -= ('A' - 10);
			else
				b -= '0';
			*dst++ = 16 * a + b;
			src += 3;
		} else if (*src == '+') {
			*dst++ = ' ';
			src++;
		} else {
			*dst++ = *src++;
		}
	}
	*dst++ = '\0';
}

void build_struct(struct input *input_, char *uri) {
	input_->path = malloc(strlen(uri) * sizeof(char));
	input_->cmd = malloc(strlen(uri) * sizeof(char));
	char *qus = strchr(uri, '?'); // replace the ? with a space
	if (qus != NULL) {
		*qus = ' ';
	}
	sscanf(uri, "%s cmd=%s", input_->path, input_->cmd);
}

struct input *decode_uri(char *src) {
	struct input *input_ = malloc(sizeof(struct input));
	build_struct(input_, src);
	remove_special_characters(input_->cmd, input_->cmd);
	return input_;
}