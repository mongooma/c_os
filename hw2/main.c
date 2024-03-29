﻿/* Process Creation and Process Management in C

Use C to implement a rudimentary interactive shell similar to that of bash.

*/

/* todos: 
1. check func in C - if a pointer is used as var, whether the value will be changed?
2. use lstat to find executable files for cmd
*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> 
#include <string.h>
#include <ctype.h>
#include <unistd.h> //for unix
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "functions.h"

#define MAX_CMD_LEN 1024 // user's input command
#define MAX_ARGV_LEN 64 // each argv in the command
#define MAX_PATH_LEN 1024 // a dir path
#define MAX_FILEPATH_LEN 1024 // a file path

int main(int argc, char ** argv) {

	setvbuf( stdout, NULL, _IONBF, 0 );

	/* before you run, set the $MYPATH in the bash shell 
		MYPATH=/usr/local/bin#/usr/bin#/bin#.


	*/

	/* use the getenv() to extract the $MYPATH environment variable */

	/* You can assume that each command read from the user will not exceed 1024 characters.
	   Further, you can assume that each argument will not exceed 64 characters,
	   but all memory must be dynamically allocated
	*/

	/* You may also assume that command-line arguments do not contain spaces.
	*/
	char * buffer = calloc(MAX_CMD_LEN + 1, sizeof(char));

	while (1) {

		argv_data argv_tuple;
		argv_tuple.argv_user = calloc(1, sizeof(char *)); /*temporarily assign 1 block for argv array*/
		argv_tuple.argv_no = calloc(1, sizeof(int));
		/* create an inﬁnite loop that repeatedly prompts a user to enter a command, parses the given command, locates the command executable, then executes the command (if found).
		*/

		/* before display the prompt, check for background processes */

		int status;
		pid_t child_pid;
		/***********/
		while(1){
			child_pid = waitpid(-1, &status, WNOHANG | WUNTRACED); // don't block it
			if(child_pid > 0){
			 	printf("[process %d terminated with exit status %d]\n", child_pid, status);
			}
			if(child_pid <= 0) break; /* break if all the termination msgs have been flushed out*/
		}
		/***********/
		/* Unknown issue: On submitty, adding this wait here will cause `child exit with status = 1` 
		 Try to insert it after last cmd executed*/

		/* 1. Output the current working directory 
			/User/Mark$ 
		*/
		char * cwd = calloc(MAX_FILEPATH_LEN, sizeof(char)); /* do this to avoid lost memories*/
		printf("%s$ ", getcwd(cwd, MAX_FILEPATH_LEN)); /* todo, TRY POSIX.1-2001 Standard, check man page*/
		free(cwd);

		/* 2. use fgets() to read in a command from the user */
		fgets(buffer, MAX_CMD_LEN, stdin);

		/* 3. extract command from buffer */

		trimStartEndSpace(buffer);

		if(strcmp(buffer, "") == 0 ){
			free(argv_tuple.argv_user); 
			free(argv_tuple.argv_no);
			/***********/
			while(1){
				child_pid = waitpid(-1, &status, WNOHANG | WUNTRACED); // don't block it
				if(child_pid > 0){
				 	printf("[process %d terminated with exit status %d]\n", child_pid, status);
				}
				if(child_pid <= 0) break; /* break if all the termination msgs have been flushed out*/
			} 
			/***********/
			continue; /* deal with single \n user input*/
		}

		if(strcmp(buffer, "exit") == 0 ){
			printf("bye\n");
			free(argv_tuple.argv_user); 
			free(argv_tuple.argv_no);
			/***********/
			while(1){
				child_pid = waitpid(-1, &status, WNOHANG | WUNTRACED); // don't block it
				if(child_pid > 0){
				 	printf("[process %d terminated with exit status %d]\n", child_pid, status);
				}
				if(child_pid <= 0) break; /* break if all the termination msgs have been flushed out*/
			}  
			/***********/
			break; /* deal with single \n user input*/
		}
		
		argv_tuple = getCmd(buffer, argv_tuple);  /* argv_user is changed after this*/
		/* must set a return value otherwise argv_tuple will be modified by returning; weird*/

		// #ifdef DEBUG
		// for(int i = 0; i < argv_tuple.argv_no[0]; i ++){
		// 	//printf("%s len: %d\n", argv_user[i], (int) strlen(argv_user[i]));
		// 	printf("%s len: %d\n", argv_tuple.argv_user[0], (int) strlen(argv_tuple.argv_user[0]));
		// }
		// #endif

		/* 4. To execute the given command, a child process is created via fork()*/

		/* 4.1 execute the commands */
		#ifdef DEBUG
		printf("main: we are at %s \n", getcwd(NULL, 0)); /* todo, TRY POSIX.1-2001 Standard, check man page*/
		#endif

		int rc = execute_cmd(argv_tuple.argv_user, argv_tuple.argv_no[0]);
		/* argv_tuple.argv_user, .argv_no are not freed within execute_cmd*/

		if(rc == -1){
			//printf("\n");
			// anything wrong with the last cmd, simply continue
		}

		for(int i = 0; i < *argv_tuple.argv_no; i ++){
		 	free(argv_tuple.argv_user[i]);
		 }
		free(argv_tuple.argv_user); 
		free(argv_tuple.argv_no); 
	}

	free(buffer);

	return EXIT_SUCCESS; /* EXIT_SUCCESS AND EXIT_FAILURE are for main() only*/
}
