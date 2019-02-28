/* test valgrind on execv*/


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

int main(){


	char ** c = calloc(2, sizeof(char *));

	c[0] = calloc(10, sizeof(char));
	strcpy(c[0], "ls");
	c[1] = NULL;

	pid_t pid = fork();

	if(pid == 0){
		execvp("ls", c);
	}

	wait(NULL);

	free(c[0]);
	free(c[1]);
	free(c);

	return 0;


}