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


	char ** argv = calloc(10, sizeof(char *) );

	//argv[0] = calloc(10, sizeof(char));
	argv[0] = "ls";
	//argv[1] = calloc(10, sizeof(char));
	argv[1] = "-l";
	//argv[2] = calloc(10, sizeof(char));
	argv[2] = NULL;


	pid_t pid = fork();
	if(pid == 0){
		execvp("ls", argv); 
	}else{
		wait(NULL);

	}

	for(int i=0; i < 3; i ++){
		free(argv[i]);
	}
	free(argv);


	return 0;


}