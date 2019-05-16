#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"

/*
Takes argc and argv from main and processes the arguments.
Sets the members of struct commandlineoptions.
Defaults are:
restrictConnections = 1
port = "3838"
Otherwise the members are set to the user-specified values.
Also provides usage and help documentation.

Returns 1 if program should continue,
Returns 0 if program should stop.
*/
int handleCommandLineOptions(int argc, char** argv, struct commandlineoptions *options)
{
	int i, j, portWasNotSet = 1, connectionRestrictionNotRemoved = 1;
	options->restrictConnections = 1;
	strcpy(options->port, "3838");

	char usage[256], errorMsg[256], seeHelp[256], help[2048];

	// Error Messages
	strcpy(errorMsg, argv[0]);
	strcat(errorMsg, ": invalid option");
	strcpy(seeHelp, "Try '");
	strcat(seeHelp, argv[0]);
	strcat(seeHelp, " --help' for more information.");

	// Usage Message
	strcpy(usage, "Usage: ");
	strcat(usage, argv[0]);
	strcat(usage, " [OPTION]...");

	// Help Page
	strcpy(help, usage);
	strcat(help, "\n"
		"Web-Based Remote Command Server\n"
		"URL: GOES!HERE\n"
		"The web server remotely runs a shell command specified by the user and displays the results.\n"
		"The web server displays a simple HTML page with the following elements:\n"
		"  A text box for getting your command\n"
		"  A button to execute the command\n"
		"  A text area to show the command that was run\n"
		"  A text area to show stdout results\n"
		"  A text area to show stderr results\n"
		"Server runs on port 3838 by default\n"
		"Server handles multiple requests at the same time\n"
		"As a security measure, the server only accepts connections from localhost by default\n\n"
		"OPTIONs:\n"
		"  -r                       allows the server to accept connections not from localhost\n"
		"  -p, --port=PORTNUMBER    use PORTNUMBER instead of the default port: 3838\n"
		"                           PORTNUMBER must be in the range 0-65535\n\n"
		"Exit status:\n"
		" 0  if OK\n"
		" 1  if #SPECIFY#\n\n"
		"Authors:\n"
		"Alex Bonasera\n"
		"Andrew Weith\n"
		"Mac Shakshober\n");

	// If there are any command line arguments then find and evaluate them.
	if (argc > 1)
	{
		// Loop through argument strings
		for (i = 1; i < argc; ++i)
		{
			// Make sure that the argument string is a valid command line argument
			// -p 'integer'
			if ((!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port")) && portWasNotSet)
			{
				// An argument was provided for option '-p'
				if (i + 1 < argc)
				{
					// Advance to the next argument string
					++i;
					// Loop through the next string to make sure it is an integer.
					for (j = 0; j < strlen(argv[i]); ++j)
					{
						// All characters must be between '0' and '9'
						if (argv[i][j] < '0' || '9' < argv[i][j])
						{
							// At least 1 character was not an integer: incorrect command line input
							fprintf(stderr, "%s non-positive integer: '%s'\n%s\n", errorMsg, argv[i], seeHelp);
							return 0;
						}
					}
					// If this code runs, a valid port was specified
					portWasNotSet = 0;
					if (atoi(argv[i]) <= 65535)
					{
						strcpy(options->port, argv[i]);
					}
					else
					{
						// At least 1 character was not an integer: incorrect command line input
						fprintf(stderr, "%s specified port is too large, use 0-65535: '%s'\n%s\n", errorMsg, argv[i], seeHelp);
						return 0;
					}
				}
				// An argument was NOT provided for option '-p'
				else
				{
					fprintf(stderr, "%s '%s' requires a positive integer as an argument\n%s\n", errorMsg, argv[i], seeHelp);
					return 0;
				}
			}
			// -r
			else if (!strcmp(argv[i], "-r") && connectionRestrictionNotRemoved)
			{
				// set flag to remove connection restriction
				connectionRestrictionNotRemoved = 0;
				options->restrictConnections = 0;
			}
			// --help
			else if (!strcmp(argv[i], "--help"))
			{
				// Print help page
				printf("%s", help);
				return 0;
			}
			// Provided a string that would indicate a filename/directory
			// '.' || '\' || '/'
			// Provide usage information
			else if (strstr(argv[i], ".") != NULL || strstr(argv[i], "\\") != NULL || strstr(argv[i], "/") != NULL)
			{
				fprintf(stderr, "%s '%s'\n%s\n%s\n", errorMsg, argv[i], usage, seeHelp);
				return 0;
			}
			// Provided an option, but it doesn't exist
			else
			{
				fprintf(stderr, "%s '%s'\n%s\n", errorMsg, argv[i], seeHelp);
				return 0;
			}
		}
	}
	// Success
	return 1;
}