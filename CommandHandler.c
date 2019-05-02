#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "execute.h"
#include "structs.h"

#define INITIAL_BUFFER_SIZE 1024

struct output execute_command(char *cmd[]);
void *handle_input(void *input);

// TODO: Implement cd command

struct output execute_command(char *cmd[]) {
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
		char *stdout = NULL;
		char *stderr = NULL;
		// Read all of pout into stdout
		do {
			stdoutsize += INITIAL_BUFFER_SIZE;
			stdout = realloc(stdout, stdoutsize * sizeof(char));
			nBytes = read(pout[0], &stdout[stdoutsize - INITIAL_BUFFER_SIZE], INITIAL_BUFFER_SIZE);
		} while (nBytes == INITIAL_BUFFER_SIZE);
		// Read all of perr into stderr
		do {
			stderrsize += INITIAL_BUFFER_SIZE;
			stderr = realloc(stderr, stderrsize * sizeof(char));
			nBytes = read(perr[0], &stderr[stderrsize - INITIAL_BUFFER_SIZE], INITIAL_BUFFER_SIZE);
		} while (nBytes == INITIAL_BUFFER_SIZE);
		wait(NULL);
		struct output output_ = { stdout, stderr };
		return output_;
	}
}

int main(int argc, char *argv[]) {
	char *args[] = { "echo", "Yay", "it", "works", NULL };
	struct output o = execute_command(args);
	printf("stdout: (%s)\n", o.stdout);
	printf("stderr: (%s)\n", o.stderr);
}