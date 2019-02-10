/* Process Creation and Process Management in C

Use C to implement a rudimentary interactive shell similar to that of bash.

*/

/* todos: 
1. check func in C - if a pointer is used as var, whether the value will be changed?
2. use lstat to find executable files for cmd
*/

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
#include "functions.h"

#define MAX_CMD_LEN 1024 // user's input command
#define MAX_ARGV_LEN 64 // each argv in the command
#define MAX_PATH_LEN 1024 // a dir path
#define MAX_FILEPATH_LEN 1024 // a file path


int main(int argc, char ** argv) {
	/* before you run, set the $MYPATH in the bash shell 
		MYPATH=/usr/local/bin#/usr/bin#/bin#.


	*/

	/* use the getenv() to extract the $MYPATH environment variable */

	/* You can assume that each command read from the user will not exceed 1024 characters.
	   Further, you can assume that each argument will not exceed 64 characters,
	   but all memory must be dynamically allocated
	*/

	/* You may also assume that command-line arguments do not contain spaces.
	*/
	char *buffer = calloc(MAX_CMD_LEN + 1, sizeof(char));
	char **argv_user = calloc(1, sizeof(char *)); /*temporarily assign 1 block for argv array*/
	char *FILE = calloc(MAX_FILEPATH_LEN, sizeof(char)); /* todo: initialize */
	int argv_no;
	int r;
	char * cwd = calloc(MAX_PATH_LEN, sizeof(char));

	strcpy(cwd, getcwd(NULL, 0)); /* RESTORE cwd when needed */

	while (1) {
		/* create an inﬁnite loop that repeatedly prompts a user to enter a command, parses the given command, locates the command executable, then executes the command (if found).
		*/

		/* 1. Output the current working directory 
			/User/Mark$ 
		*/
		printf("%s$ ", getcwd(NULL, 0)); /* todo, TRY POSIX.1-2001 Standard, check man page*/

		/* 2. use fgets() to read in a command from the user */
		fgets(buffer, MAX_CMD_LEN, stdin);
		/* 3. extract command from buffer */

		trimEndSpace(buffer);
		
		argv_no = getCmd(buffer, argv_user);  /* argv_user is changed after this*/


		/* 4. To execute the given command, a child process is created via fork()*/

		/* 4.1 special case cd, exit*/

		if (strcmp(argv_user[0], "cd") == 0 /* todo: check string comparison*/) {

			/* change directory in the parent process*/

			if (argv_no < 2) { chdir(getenv("HOME")); /* as requested in the hw2 pdf*/}
			if (argv_no > 2) { printf("man cd; cd <dir_name>\n"); }

			r = chdir(argv_user[1]); 
			if (r == -1){perror("");}

			continue;

		}

		if (strcmp(argv_user[0], "exit") == 0 ){

			/* exit parent process*/

			if (argv_no > 2) {perror("man exit\n");	continue; }
			return 0; 

		}


		/* 4.2 check in $MYPATH directories for executable for user cmd*/

		if(getenv("MYPATH") == NULL){
			perror("Specify the $MYPATH variable using: \n"
				"bash$ export MYPATH=/usr/local/bin#/usr/bin#/bin#. \n"
				"bash$ echo $MYPATH \n"
				"MYPATH=/usr/local/bin#/usr/bin#/bin#. \n"
				"bash$ unset MYPATH \n"
					"");
		}

		strcpy(FILE, searchPath(getenv("MYPATH"), argv_user[0]));/* search $MYPATH */;
		chdir(cwd); /* this is to deal with the side-effect of using searchPath() */
		/* eg. '/usr/bin/sudo' */


		/* 4.3 create a child process to execute the command using executable file found */
		/* May want to check this: https://submitty.cs.rpi.edu/index.php?semester=s19&course=csci4210&component=misc&page=display_file&dir=course_materials&file=fork-with-exec.c&path=%2Fvar%2Flocal%2Fsubmitty%2Fcourses%2Fs19%2Fcsci4210%2Fuploads%2Fcourse_materials%2Flec-01-28%2Ffork-with-exec.c*/
		if ( FILE == NULL/* search directory; command not found*/) { perror("Command not found. \n"); continue; /* take in another user's input */}

		execute_cmd(argv_user, argv_no);
		
	}

	return EXIT_SUCCESS; /* EXIT_SUCCESS AND EXIT_FAILURE are for main() only*/
}
