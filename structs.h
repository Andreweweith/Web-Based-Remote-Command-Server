#ifndef STRUCTS_H
#define STRUCTS_H

struct input {
	char* path;
	char* cmd;
};

struct output {
	char* stdout_;
	char* stderr_;
};

struct httpheader {
	char* firstline;
	char* data;
	struct httpheaderitem* item;
};

struct httpheaderitem {
	char* key;
	char* val;
	struct httpheaderitem* next;
};

struct commandlineoptions {
	char port[256];
	int restrictConnections;
};

#endif