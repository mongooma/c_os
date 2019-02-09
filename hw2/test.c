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

	int pid = fork()

	if ( pid == -1 ) { /* child process creation failed*/

		perror("fork() failed \n");
		return EXIT_FAILURE;

	}
	else if( pid == 0){

		//execvp("/usr/bin/ls", ["ls", NULL]);
		execlp("/usr/bin/ls", "ls", NULL)

	}
	else {
		/*PARENT PROCESS*/ 
		int status;
		pid_t child_pid;

		while (1) {
			child_pid = waitpid(pid, &status, WNOHANG); 
			/*  waitpid():  on  success, returns the process ID of the child whose state has changed; if WNOHANG was speci‐
       fied and one or more child(ren) specified by pid exist, but have not yet changed state, then 0 is returned.
       On error, -1 is returned.*/
			if (child_pid != 0) break;
			sleep(1);
		}

		
		if (child_pid == -1) {
			printf("waitpid() failed for child process %d \n", pid);
		}
	}
}
