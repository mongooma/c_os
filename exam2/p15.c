#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SHARED_KEY 8000
int main( int argc, char * argv[] ){
	int shmid = shmget( SHM_SHARED_KEY, sizeof( int * ), IPC_CREAT | 0660 );
	int * data = shmat( shmid, NULL, 0 );

	int pid = fork();
	
	if ( pid > 0 ) waitpid( pid, NULL, 0 ); // block

	int i, stop = 6;for ( i = 1 ; i <= stop ; i++ ){data[i%2] += i;}

	printf( "%s: data[%d] is %d\n", pid > 0 ? "PARENT" : "CHILD", 0, data[0] );
	printf( "%s: data[%d] is %d\n", pid > 0 ? "PARENT" : "CHILD", 1, data[1] );

	shmdt( data );

	return EXIT_SUCCESS;
}