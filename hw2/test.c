#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h> 
#include <string.h>
#include <ctype.h>
#include <unistd.h> //for unix
#include <errno.h>
#include <sys/wait.h>
/*********LOCAL********/
#include "functions.h"




int main_(){

	pid_t pid = fork();

	
	if( pid == 0){

		char ** arg = calloc(1, sizeof(char *));
		arg[0] = strcpy(arg[0], "ls");
		//char * arg [] = {"ls", NULL};
		for (int i=0; i < 2; i ++){

			printf("%s \n", arg[i]);
			fflush(stdout);
		}
		arg = realloc(arg, 2 * sizeof(char *)); 
		arg[1] = NULL;

		execv("ls", arg);

	}
	else {
		/*PARENT PROCESS*/ 
		int status;
		pid_t child_pid;

		while (1) {
			child_pid = waitpid(pid, &status, WNOHANG); 
			if (child_pid != 0) break;
			sleep(1);
		}

		if (child_pid == -1){perror("");}
	}
}

int main(){

	char ** argv = calloc(1, sizeof(char *));
	getCmd("ls here im", argv);

	free(argv);



}
