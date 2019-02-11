#pragma once // check https://en.wikipedia.org/wiki/Include_guard for more info

int * trimEndSpace(char * buffer);

int getCmd(const char * buffer, char ** argv);

char * searchPath(const char * PATH, const char * cmd);

int searchSinglePath(const char * p, const char * s);

int execute_cmd(char ** argv, const int argv_no, const char * cwd);


