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

#define MAX_CMD_LEN 1024 // user's input command
#define MAX_ARGV_LEN 64 // each argv in the command
#define MAX_PATH_LEN 1024 // a dir path
#define MAX_FILEPATH_LEN 1024 // a file path


/*In C and C++ programming, arrays are always passed to funcs by pointer*/
int * trimEndSpace(char * buffer) {
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
	return 0;
}


int getCmd(const char * buffer, char ** argv) {
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
			
			argv[k] = calloc(strlen(arg_buffer) + 1, sizeof(char));
			strcpy(argv[k], arg_buffer); 
			
			k++; // for argv
			argv = realloc(argv, (k+1) * sizeof(char *)); /* extend one block for argv, the new block is not initialized*/
			j = 0;

		}
		else {
			
			arg_buffer[j] = buffer[i];
			j++; // for argv_buffer

		}
		i++; 
	}
	/*dealing with the last argument*/
	arg_buffer[j] = '\0';
	argv[k] = calloc(strlen(arg_buffer) + 1, sizeof(char)); 
	strcpy(argv[k], arg_buffer); 

	free(arg_buffer);
	
	return k+1;
}


char * searchPath(const char * PATH, const char * cmd) {
	/* debugged */
	/* return char * p for the full path for the executable cmd 
		NULL if no executable found*/

	/* search in $MYPATH to find the executable FILE*/

#ifdef DEBUG_pass
	FILE * f = fopen("./log.txt", "w");
	fprintf(f, "PATH: %s \n", PATH);
#endif //debug
	
	int searchSinglePath(const char * p, const char * s) {
		/* debugged 
			return 0 if an executable s is found;
				1 if an not-executable s is found;
				-1 if others
		Search the directory specified by p for file specified by s
		side-effect:
			after this function the working directory of the parent process will be changed*/

#ifdef DEBUG_pass
		fprintf(f, "%s\n", p);
#endif //debug

		chdir(p);

		DIR * dir = opendir(p);   /* open the directory specified by p*/
		if (dir == NULL)
		{
			perror("opendir() failed");
			return -1;
		}
		/* DIR is a directory stream for reading the directory */
#ifdef DEBUG_pass
		int erronum;
#endif
		/* */

		/* check directories.c https://submitty.cs.rpi.edu/index.php?semester=s19&course=csci4210&component=misc&page=display_file&dir=course_materials&file=directories.c&path=%2Fvar%2Flocal%2Fsubmitty%2Fcourses%2Fs19%2Fcsci4210%2Fuploads%2Fcourse_materials%2Flec-01-31%2Fdirectories.c */

		struct dirent * file;
		struct stat buf;
		int rc;

		while ((file = readdir(dir)) != NULL) /* inspect every file under dir*/
		{
			rc = lstat(file->d_name, &buf);

			if (rc == -1)
			{
				printf("%s,\n", file->d_name);
				//fprintf(stderr, "Value of errno: %d\n", errno);
	      		perror("Error printed by perror");
				continue; /* ignore some broken file*/
			}

			if (strcmp(file->d_name, s) == 0) 
			{
				if (S_IXUSR & buf.st_mode) { /* S_IXUSR: macro mask value; 
												if the owner has exec permission
												*/

					//printf(" -- exe file\n");
					return 0;
				}else{

					printf("No exec permission \n");
					return 1;
				}
			}

		}
		/* nothing related is found*/
		closedir(dir);
		return -1;
	}


	char * p = calloc(MAX_PATH_LEN, sizeof(char));
	int i = 0;
	int j = 0;
	int FOUND = -1;

	while (PATH[i] != '\0') {
		/*todo: assume the maximum length for $MYPATH is 1024*/
		p[j] = PATH[i];

		if ((PATH[i] == ':') || (PATH[i] == '#')) {  /* if a complete path is stored*/
			/* in hw2 pdf the separator is '#'*/
			/* TODO: using PATH[i] == '#' || ':' won't give the right result*/

			p[j] = '\0';
#ifdef DEBUG_pass
		fprintf(f, "%s\n", p);
#endif //debug
			
			FOUND = searchSinglePath(p, cmd);

			/*  0 if an executable s is found;
				1 if an not-executable s is found;
				NULL if other errors*/

			if (FOUND == 0) { /* An executable cmd is found under p*/
				p[j] = '/';
				j ++;
				while(*cmd != '\0') {p[j] = *cmd; cmd ++; j ++;} /* copy cmd to the end of the p path*/
				p[j] = '\0';

				return p;

			}else{ /* start searching another dir in PATH*/
				j = 0; 
			}
		}
		else { /* continue reading from PATH*/

			j++;

		}

		i++;
	}
	/*if nothing's found*/
	free(p);
	return NULL;
}



int execute_cmd(char * argv, int argv_no){

			argv_user = realloc(argv_user, (argv_no + 1) * sizeof(char *)); /* extend one block for argv, the new block is not initialized*/
			argv_user[argv_no] = NULL ;
#ifdef DEBUG
			printf("cmd no.: %d \n", argv_no + 1);
			for(int i=0; i < argv_no + 1; i ++){
				printf("%s, \n", argv_user[i]);
			}
#endif
			
			execvp(FILE, argv_user); /* argv of type char * const *, 
									if initialize use char * const cmds [] = {"sudo", "apt-get", "install", NULL};
									not char * const cmds [] = ... */


		/* within a child process, no fork() is executed here*/

		if ( strcmp(argv[argv_no], "&") != 0){/* if foreground

			(parent) (wait)
			   |
			   |
			child (exec)

		*/


		}else{  }

		/* if background

			(parent) (wait)
			   |
			   |
			child (wait)
			   |
			   |	 
			child_1 (exec)	 
		*/



		/* if with one pipe
			
			(parent) (wait)
			   |
			   |
			child (wait)
			 /  \
			/	 \
		child_1	 child_2
		(stdout --> stdin)

		*/



		return 0;




}




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
		
		pid_t pid = fork(); /* each time execute one command*/
		
		if ( pid == -1 ) { /* child process creation failed*/ perror("fork() failed \n"); return EXIT_FAILURE;
		}
		else if( pid == 0){
			/*CHILD PROCESS*/
			/* calling execvp() to execute the command */
			/* man 3 execvp: 
			 The execlp(), execvp(), and execvpe() functions duplicate the actions of the shell in  searching
       for  an  executable  file if the specified filename does not contain a slash (/) character.
			If  the specified filename includes a slash character, then PATH is ignored, and the file at the
       specified pathname is executed.*/

			execute_cmd(argv_user, argv_no);

		}
		else {
			/*PARENT PROCESS*/ 
			int status;
			pid_t child_pid;

			if ( strcmp(argv_user[argv_no-1], "&") != 0  /*foreground processing*/) {
				while (1) {
					child_pid = waitpid(pid, &status, WNOHANG); /* wait in the parent process for the child process to finish*/
					if (child_pid != 0) break;/*-1 on error; 0 on not changed; pid on success*/
					sleep(1);
				}

				
				if (child_pid == -1) {
					printf("waitpid() failed for child process %d \n", pid);
				}

#ifdef DEBUG
				printf("cmd executed finished\n"); 

#endif
				continue;
			}
			else { /* background processing */
				/* report a child process has been created */
				printf("[running background process \"%s\"]\n", argv_user[0]);

				continue; //parent process don't wait for the child process to terminate
				
				/* todo: use waitpid() for background process*/

				/* todo: display [process 9335 terminated with exit status 0] when the background process is terminated*/
			
			}
		}
	}

	return EXIT_SUCCESS; /* EXIT_SUCCESS AND EXIT_FAILURE are for main() only*/
}
