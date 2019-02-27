#pragma once // check https://en.wikipedia.org/wiki/Include_guard for more info

typedef struct {

	int * argv_no;
	char ** argv_user;

}argv_data;

char * get_env_p(char * env_v);
		 
int trimStartEndSpace(char * buffer);

argv_data getCmd(const char * buffer, argv_data d);

int searchSinglePath(const char * p, const char * s);

char * searchPath(const char * PATH, const char * cmd);

int execute_cmd(char ** argv, int argv_no);

int parsePipe(int argv_no, char ** argv, char ** new_arg, int * new_arg_len);

int exec_pipe_2(char ** argv, int argv_no, char ** new_arg, int new_arg_len, pid_t * pid_1, pid_t * pid_2);

int exec_(char ** argv, int argv_no, pid_t * pid, int * p, int pipe_pos);