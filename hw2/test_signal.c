#include <signal.h>

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


/*

SIGHUP
1
Hang up detected on controlling terminal or death of controlling process

SIGINT
2
Issued if the user sends an interrupt signal (Ctrl + C)

SIGQUIT
3
Issued if the user sends a quit signal (Ctrl + D)

SIGFPE
8
Issued if an illegal mathematical operation is attempted

SIGKILL
9
If a process gets this signal it must quit immediately and will not perform any clean-up operations

SIGALRM
14
Alarm clock signal (used for timers)

SIGTERM
15
Software termination signal (sent by kill by default)

use kill -l to see a full list of signals
	

*/

/* 
	signal() is for catching a signal;
	kill() is for sending a signal;
	if a child want to send a msg to parent about its status, then it 
	has to call kill() on itself;


	when a signal is catched, before the handler is returned, the process will be 
	interrupted;

*/


typedef void (*sighandler_t)(int);

void handler(){

	printf("do nothing \n");

}

int main(){


	for(int i = 0; i < 5; i++){
		printf("%d, \n", i);
		pid_t pid = fork();

		if(pid == 0){

			while(1){
				kill(getpid(), SIGINT); /* commit suicide*/
				printf("Still alive! \n");
			}

		}else{
			/* see if the parent catches it*/
			int status;
			pid_t child_pid;

			while(1){
				child_pid = waitpid(0, &status, WNOHANG);
				if(child_pid != 0) break;
			}

			//printf("Let the parent terminate the child process \n");
			//kill(pid, SIGINT);

		}
	}

	printf("Parent is gonna terminate. \n");

	
	sleep(3);
	return EXIT_SUCCESS;

}