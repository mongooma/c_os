#pragma once // check https://en.wikipedia.org/wiki/Include_guard for more info

typedef struct {

	int * argv_no;
	char ** argv_user;

}argv_data;

int * trimEndSpace(char * buffer);

argv_data getCmd(const char * buffer, argv_data d);

char * searchPath(const char * PATH, const char * cmd);

int searchSinglePath(const char * p, const char * s);

int execute_cmd(char ** argv, const int argv_no, const char * cwd);


