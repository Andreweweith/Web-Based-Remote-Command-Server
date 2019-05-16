## Team-Physics

Members: Andrew Weith, Alex Bonasera, Mac Shakshober

[Alex Bonasera](https://github.com/Derproid)

[Andrew Weith](https://github.com/Andreweweith)

[Mac Shakshober](https://github.com/macshakshober)

## Web-based Remote Command Server

**Name:** wrc-server - start a web-based remote command server.

**Synopsis:** wrc-server [OPTION]...

**Description:** The web server displays a simple HTML page where the user enters a shell command and presses `Run`. The command to be run is displayed in a text box. The web server then remotely executes the command and displays the `stdout` and `stderr` each in its own text box. The server runs on port 3838 by default and handles multiple requests at the same time. As a security measure, the server only accepts connections from localhost by default.

**OPTIONs:**

+ -r allows the server to accept connections from sources other than localhost

+ -p, --port=PORTNUMBER use PORTNUMBER instead of the default port: 3838

+ --help provide the user with a documentation page

**Additional Functionalities**

1. Providing the `-p` option followed by a positive integer in the range 0-65535 will change the port that the server is run on.

2. The server allows asynchronous submission of the command to the server and display of results to the user without needing to reload the page.

3. Providing the `--help` option prints a help page for the user.

[Progress Report](progress.md)
