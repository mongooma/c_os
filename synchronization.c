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

#include <sys/ipc.h>
#include <sys/shm.h>

/* */
#define SHM_SHARED_KEY 8192

int child(int j, int k, char * data){

	for(j <= k){

		data[j] = tolower(data[j]);
		j ++;
	}

	/* detach from the shared mem segment */
	int rc = shmdt( data );

	sleep(4);

	return 0;

}

void parent(int children, char * data, int shmid){

	int status;
	pid_t child_pid = -1;

	for(int i = 0; i < 4; i ++){
		if(child_pid != 0){
			child_pid = fork();
			if(child_pid == 0){
				child(0, 4, data);
			}
		}
	}

	for(int i = 0; i < 4; i++){
		wait(&status);
	}

	 

	int rc = shmdt( data );




}

int main(){

	/* CREATE */
	key_t key = SHM_SHARED_KEY;
	int shmid = shmget(key, 1024, IPC_CREAT | 0660 ); //  allocates a System V shared memory segment
	/* On success, a valid shared memory identifier is returned.  On error, -1 is returned */

	/* ATTACH TO IT*/

	char * data = shmat(shmid, NULL, 0); 
	/*  if shmaddr  is NULL, the system chooses a suitable (unused) page-aligned address to attach
          the segment 

        On  success,  shmat()  returns  the  address  of the attached shared memory segment; on error,
       (void *) -1 is returned,*/









}