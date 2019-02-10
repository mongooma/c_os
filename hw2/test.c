#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h> 
#include <string.h>
#include <ctype.h>
#include <unistd.h> //for unix
#include <errno.h>




int main(){

	int pid = fork();

	
	if( pid == 0){

		int pid_1 = fork();

		if( pid_1 == 0 ){

			execlp("/usr/bin/ls", "ls", NULL)

		}else{

			int status_1;
			pid_t child_pid_1;

			while (1) {
				child_pid_1 = waitpid(pid_1, &status, WNOHANG); 
				if (child_pid_1 != 0) break;
				sleep(1);
			}

		}


	}
	else {
		/*PARENT PROCESS*/ 

	
	}
}
