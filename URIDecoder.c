#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "URIDecoder.h"
#include "structs.h"

// Credit to https://stackoverflow.com/a/14530993
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
		}
		else if (*src == '+') {
			*dst++ = ' ';
			src++;
		}
		else {
			*dst++ = *src++;
		}
	}
	*dst++ = '\0';
}

void build_struct(struct input *input_, char *uri) {
	char *key;
	char *val;
	input_->path = strtok(uri, "?");
	while ((key = strtok(NULL, "=")) != NULL) {
		val = strtok(NULL, "&");
		if (!strcmp(key, "uid")) {
			input_->uid = atoi(val);
		} else {
			input_->cmd = val;
		}
	}
}

struct input *decode_uri(char *src) {
	struct input *input_ = malloc(sizeof(struct input));
	char *dst = malloc(strlen(src) + 1);
	remove_special_characters(dst, src);
	build_struct(input_, dst);
	return input_;
}