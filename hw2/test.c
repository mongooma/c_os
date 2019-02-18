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



int pipe_test(){

	/* */
	/* todo: pratice on pipe and waitpid --> monitor the status changes by using printf in the while(1) loop*/
	int p[2];
	int rc = pipe(p);
	if (rc == -1){perror("failed! \n");}	

	
	pid_t pid_1 = fork();
	
	if( pid_1 == 0){
		close(p[0]); // 0 is read
		close(1); // close stdout

		dup2(p[1], 1);
		close(p[1]); // close write after finish

		execlp("/bin/ps", "ps", "-ef", NULL);
		//execlp("/bin/cat", "cat", "functions.c", NULL);
		perror("failed: here1 \n");

	} 

	pid_t pid_2 = fork(); // fork another child process from parent
	if (pid_2 == 0){
		close(p[1]); // close write
		close(0); // close stdin

		dup2(p[0], 0); // read is using stdin's address
		close(p[0]); //close after read finished

		execlp("/bin/grep", "grep", "root", NULL);
		//execlp("/usr/bin/wc", "wc", "-l", NULL);
		//execlp("/bin/more", "more", NULL);
		perror("failed: here2 \n");

	}


	/* at parent process */
	int status1, status2, status;
	pid_t child_pid, child_pid_1, child_pid_2;

	close(p[0]); // notice that any process having the copy of pipe could read&write to the buffer
	close(p[1]);

	while (1) {
		/* worked*/
		// child_pid = waitpid(0, &status, WNOHANG);  
		// if (child_pid != 0) break; //
		/**********/
		// wait for pid_1 and pid_2
		child_pid_1 = waitpid(pid_1, &status1, 0);  
		child_pid_2 = waitpid(pid_2, &status2, 0);  
		// printf("1: %d, 2: %d \n", child_pid_1, child_pid_2);
		if ((child_pid_1 != 0) && (child_pid_2 != 0)) break; //
		sleep(1);
	}

	printf("pid_1: %d, pid_2: %d,   %d terminated! \n", pid_1, pid_2, child_pid);
	// if ((child_pid_1 == -1) || (child_pid_2 == -1)){ perror("waitpid failed! ");}

	return 0;

}




int main(){


	pipe_test();

	return 0;


}
