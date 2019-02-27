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

	argv[0] = calloc(10, sizeof(char));
	strcpy(argv[0], "ls");

	argv[1] = calloc(10, sizeof(char));
	strcpy(argv[1], "-l");

	argv[2] = calloc(10, sizeof(char));
	argv[2] = NULL;

	pid_t pid = fork();

	if(pid == 0){

		char ** argv_cp = calloc(10, sizeof(char *) );
		
		for(int i=0; i < 2; i ++){
			argv_cp[i] = strdup(argv[i]); // the memory are allocated using malloc ...
			free(argv[i]); // this line will cause error
		}
		argv_cp[2] = NULL;
		free(argv[2]);

		free(argv);

		execvp("ls", argv_cp); 

	}else{

		wait(NULL);

	}

	for(int i=0; i < 3; i ++){
		free(argv[i]);
	}
	free(argv);


	return 0;


}