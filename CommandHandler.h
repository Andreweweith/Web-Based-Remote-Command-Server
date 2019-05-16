#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

struct output *handle_command(char *cmd);
struct output *execute_command(char *cmd[]);

#endif