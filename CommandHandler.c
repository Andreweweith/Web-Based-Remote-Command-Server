#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "structs.h"
#include "CommandHandler.h"

#define INITIAL_BUFFER_SIZE 1024

struct output *handle_command(char *cmd) {
	int arg_count = 2;
	char *chr;
	for (chr = cmd; *chr != '\0'; chr++) {
		if (*chr == ' ') {
			arg_count++;
		}
	}
	char *cmds[arg_count];
	int i = 0;
	char *tok;
	char *saveptr;
	for (tok = strtok_r(cmd, " ", &saveptr); tok != NULL; tok = strtok_r(NULL, " ", &saveptr)) {
		cmds[i++] = tok;
	}
	cmds[arg_count - 1] = NULL;
	return execute_command(cmds);
}

struct output *execute_command(char *cmd[]) {
	pid_t pid;
	int pout[2];
	int perr[2];
	if (pipe(pout) == -1 || pipe(perr) == -1) {
		perror("pipe");
		exit(-1);
	}
	if ((pid = fork()) == -1) {
		perror("fork");
		exit(-1);
	}
	if (pid == 0) {
		dup2(pout[1], STDOUT_FILENO);
		dup2(perr[1], STDERR_FILENO);
		close(pout[0]);
		close(pout[1]);
		close(perr[0]);
		close(perr[1]);
		execvp(cmd[0], cmd);
		perror(cmd[0]); // This is reached only if an error occurs in exec (not the program it runs)
		exit(-1);
	} else {
		close(pout[1]);
		close(perr[1]);
		int nBytes;
		int stdoutsize = 0;
		int stderrsize = 0;
		char *stdout_ = NULL;
		char *stderr_ = NULL;
		// Read all of pout into stdout
		do {
			stdoutsize += INITIAL_BUFFER_SIZE;
			stdout_ = realloc(stdout_, stdoutsize * sizeof(char));
			nBytes = read(pout[0], &stdout_[stdoutsize - INITIAL_BUFFER_SIZE], INITIAL_BUFFER_SIZE);
		} while (nBytes == INITIAL_BUFFER_SIZE);
		// Read all of perr into stderr
		do {
			stderrsize += INITIAL_BUFFER_SIZE;
			stderr_ = realloc(stderr_, stderrsize * sizeof(char));
			nBytes = read(perr[0], &stderr_[stderrsize - INITIAL_BUFFER_SIZE], INITIAL_BUFFER_SIZE);
		} while (nBytes == INITIAL_BUFFER_SIZE);
		waitpid(pid, NULL, 0);
		struct output *output_ = malloc(sizeof(struct output));
		output_->stdout_ = stdout_;
		output_->stderr_ = stderr_;
		return output_;
	}
}