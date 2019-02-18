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

#define MAX_CMD_LEN 1024 // user's input command
#define MAX_ARGV_LEN 64 // each argv in the command
#define MAX_PATH_LEN 1024 // a dir path
#define MAX_FILEPATH_LEN 1024 // a file path

argv_data getCmd(const char * buffer, argv_data d) {
	/* 	:param const char * buffer 	user input
		:param argv_data d 			a struct to store separate cmds
		:return argv_data d
	*/
	/* read buffer and store each argument in char ** argv*/
	/* argv has initial 1 block and will be extended after this function:
		*/
	/* 
		Output: length of argv k+1
	*/

	/* debugged 30min */

	char *arg_buffer = calloc(MAX_ARGV_LEN + 1, sizeof(char)); /* temp, freed after*/

	int i = 0; 
	int j = 0;
	int k = 0;
	
	while (buffer[i] != '\0') {
		if (isspace(buffer[i])) {
			
			arg_buffer[j] = '\0';
			d.argv_user[k] = calloc(strlen(arg_buffer) + 1, sizeof(char));
			strcpy(d.argv_user[k], arg_buffer); 
			
			k++; // for argv
			d.argv_user = realloc(d.argv_user, (k+1) * sizeof(char *)); /* extend one block for argv, the new block is not initialized*/
			j = 0;

		}
		else {
			
			arg_buffer[j] = buffer[i]; /* bad pointer assignment (?)*/
			j++; // for argv_buffer

		}
		i++; 
	}
	/*dealing with the last argument*/

	arg_buffer[j] = '\0';
	d.argv_user[k] = calloc(strlen(arg_buffer) + 1, sizeof(char)); 
	strcpy(d.argv_user[k], arg_buffer); 

	free(arg_buffer); //this free will not change argv

	#ifdef DEBUG
	for(int i = 0; i < k+1; i ++){
		printf("%s len: %d\n", d.argv_user[i], (int) strlen(d.argv_user[i]));
	}
	#endif

	d.argv_no[0] = (int)(k+1);

	return d; // actual number of argv
}

int main(){

	char * buffer = "ls -l &";
	argv_data d; 
	d.argv_user = calloc(1, sizeof(char *)); /*temporarily assign 1 block for argv array*/
	d.argv_no = calloc(1, sizeof(int));
	d = getCmd(buffer, d);
	for(int i=0; i < *d.argv_no; i ++){
		printf("%s \n", d.argv_user[i]);
	}	

	return 0;

}