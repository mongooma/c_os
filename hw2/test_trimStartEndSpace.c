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
#define MAX_FILEPATH_LEN 1024 // a file pa


int trimStartEndSpace(char * buffer) {
	/* ATTENTION: We are going to change buffer here*/
	/* buffer has to be in dynamically allocated memory*/
	/* the buffer is trimmed*/

	/* debugged 20min...*/
	
	for (int i = strlen(buffer)-1; i > 0; i--) {
		if (isspace(buffer[i])) { /* somehow ==' ' is not effective, check for all whitespaces */
			buffer[i] = '\0';
		}
		else {
			break;
		}

	}

	for (int i = 0; i < strlen(buffer); i++) {
		if (isspace(buffer[i])) { /* somehow ==' ' is not effective, check for all whitespaces */
			strcpy(buffer, buffer + (i+1)); 
			i = 0;
		}
		else {
			break;
		}

	}
	return 0;
}

int main(){

	char * buffer = calloc(MAX_ARGV_LEN, sizeof(char));
	strcpy(buffer, "   ls -l &    ");
	trimStartEndSpace(buffer);

	printf(".%s.\n", buffer);


}