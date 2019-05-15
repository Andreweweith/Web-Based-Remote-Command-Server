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

const char cmd_string[4] = "cmd";
const char qus_string[2] = "?";
const char eql_string[2] = "=";
void build_struct(struct input *input_, char *uri) {
	char *key;
	char *val;
	char *cmd_;
	int cmd_length = 1;
	input_->path = strtok(uri, qus_string);
	strtok(NULL, eql_string);
	input_->cmd = strtok(NULL, '\0');
	/*while ((key = strtok(NULL, eql_string)) != NULL) {
		val = strtok(NULL, amp_string);
		if (!strcmp(key, uid_string)) {
			input_->uid = atoi(val);
		} else {
			cmd_ = val;
		}
	}*/
	// split single string command into string array by spaces
}

struct input *decode_uri(char *src) {
	struct input *input_ = malloc(sizeof(struct input));
	char *dst = malloc(strlen(src) + 1);
	remove_special_characters(dst, src);
	build_struct(input_, dst);
	return input_;
}