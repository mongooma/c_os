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

/*In C and C++ programming, arrays are always passed to funcs by pointer*/
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


	#ifdef DEBUG
	for(int i = 0; i < k+1; i ++){
		printf("%s len: %d\n", d.argv_user[i], (int) strlen(d.argv_user[i]));
	}
	#endif

	d.argv_no[0] = (int)(k+1);

	free(arg_buffer); //this free will not change argv
	return d;
}


int searchSinglePath(const char * p, const char * s) {
	/* debugged 
		return 0 if an executable s is found;
			1 if an not-executable s is found;
			-1 if others
	Search the directory specified by p for file specified by s */

	#ifdef DEBUG
		printf("searchSinglePath: path %s\n", p);
	#endif //debug

	char * cwd = calloc(MAX_PATH_LEN, sizeof(char));
	strcpy(cwd, getcwd(NULL, 0));

	chdir(p); // somehow we need to be under this path

	DIR * dir = opendir(p);   /* open the directory specified by p*/
	if (dir == NULL)
	{
		perror("opendir() failed");
		return -1;
	}
	/* DIR is a directory stream for reading the directory */
	#ifdef DEBUG
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
			if (S_IXUSR & buf.st_mode == 0) { /* S_IXUSR: macro mask value; 
											if the owner has exec permission										*/
				printf("No exec permission \n");
				chdir(cwd);
				return 1;
			}else{
				chdir(cwd);
				return 0;
			}
		}

	}
	/* if nothing related is found*/

	closedir(dir);
	chdir(cwd);

	return -1;
}

char * searchPath(const char * PATH, const char * cmd) {
	/* debugged */
	/* return char * p for the full path for the executable cmd 
		NULL if no executable found*/

	/* search in $MYPATH to find the executable FILE*/

	#ifdef DEBUG
		FILE * f = fopen("./log.txt", "w");
		fprintf(f, "PATH: %s \n", PATH);
	#endif //debug
	
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
	#ifdef DEBUG
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
	#ifdef DEBUG
		printf("debug:cmd not found \n");
		fflush(stdout);
	#endif
	return "nan";
}



int execute_cmd(char ** argv, int argv_no){
	/* 	param: argv, an array of strings, i.e {"ls", "-l"} not null terminated
		param: argv_no, number of argvs
		return: 
	*/
	/* master pipeline fro */
	/* to debug */

	/* At parent process */

	/* 4.2.1 special case cd, exit (taking care of at "main")*/
	#ifdef DEBUG
		printf("in execute_cmd: argv[0] %s \n", argv[0]); 
		printf("strcmp argv[0], 'cd', %d \n", strcmp(argv[0], "cd"));
		printf("argv_no, %d \n", argv_no);
		printf("execute_cmd: we are at %s \n", getcwd(NULL, 0)); /* todo, TRY POSIX.1-2001 Standard, check man page*/

	#endif

	if (strcmp(argv[0], "cd") == 0 /* todo: check string comparison*/) {

		/* change directory in the parent process*/

		if (argv_no < 2) { chdir(getenv("HOME")); /* as requested in the hw2 pdf*/}
		if (argv_no > 2) { printf("man cd; cd <dir_name>\n"); }

		int r = chdir(argv[1]);
		
		if(r == -1){
			perror("Error returned by perror: ");
		}

		return 0;

	}


	/************************/

	/* handling normal kind of cmds */

	char ** new_arg = calloc(argv_no, sizeof(char *));
	int new_arg_len = -1;

	int p = parsePipe(argv_no, argv, new_arg, &new_arg_len);

	if( p == -1 ){perror(" pipe abnormal ! \n"); exit(-1);} /* pipe cmd abnormal */

	#ifdef DEBUG
		printf("with pipe: %d \n", p);
	#endif

	if(	p == 1 ){ /* with pipe */
		pid_t pid_1, pid_2;
		#ifdef DEBUG
		printf("execute_cmd: before pipe exec, we are at %s \n", getcwd(NULL, 0)); /* todo, TRY POSIX.1-2001 Standard, check man page*/
		#endif
		exec_pipe_2(argv, argv_no, new_arg, new_arg_len, &pid_1, &pid_2); // special pipe case for hw2 
		/* will return in parent process*/

		#ifdef DEBUG
		printf("pid_1: %d, pid_2: %d, here \n", pid_1, pid_2);
		#endif

	}else{ /*without pipe */
		pid_t pid;
		#ifdef DEBUG
		printf("execute_cmd: before exec, we are at %s \n", getcwd(NULL, 0)); /* todo, TRY POSIX.1-2001 Standard, check man page*/
		#endif
		exec_(argv, argv_no, &pid);
		/* will return in parent process*/;

		#ifdef DEBUG
		printf("pid: %d, here \n", pid);
		#endif

	}

	free(argv);
	free(new_arg);
	return 0;

}

int parsePipe(int argv_no, char ** argv, char ** new_arg, int * new_arg_len){
	/*  params:
		argv,
		argv_no,
		new_arg, will not be NULL terminated (not to confuse later execution)
		new_arg_len,

		return: p == 0, no pipe; p == 1 with pipe (one '|' for hw2)

	*/


	int p = 0;
	int i = 0;
	int j = 0;

	while(i < argv_no){
		int rc = strcmp(argv[i], "|");
		if (rc == 0 ){ /* detect '|'*/
			p = 1;
			argv[i] = NULL;  /* NULL could stop execv from parsing the rest cmd*/
			if (i == argv_no - 1){perror("Nothing after pipe: \n"); return -1;}
			i++;

		}

		#ifdef DEBUG
		printf("parsePipe: %s \n", argv[i]); 
		fflush(stdout);
		#endif

		if (p == 1){
			new_arg[j] = calloc(MAX_ARGV_LEN, sizeof(char));
			new_arg[j] = strcpy(new_arg[j], argv[i]);
			j++;
		}

		i++;
	}
	*new_arg_len = j;

	return p;

}


int exec_pipe_2(char ** argv, int argv_no, char ** new_arg, int new_arg_len, pid_t * pid_1, pid_t * pid_2){
	/* to debug */

	/* 	param: pid_1 pid of the first child process
		param: pid_2 pid of the second child process
		param: argv, in the format {"ls", "-l", NULL, "wc", "-l"} anything after NULL will be discarded
		param: argv_no, in the case {"ls", "-l", NULL, "wc", "-l"} argv_no == 5
		param: new_arg, in the case {"ls", "-l", NULL, "wc", "-l"} will be {wc, -l}, not NULL terminated
		param: new_arg_no, in the case {"ls", "-l", NULL, "wc", "-l"} will be 2
		return: 

	*/
	

	/* if with one pipe
		
	    parent (wait)
		 /  \
		/	 \
	child_1	 child_2
	(stdout --> stdin)

	*/
	char * FILE = calloc(MAX_FILEPATH_LEN, sizeof(char)); 

	int p[2];
	int rc = pipe(p);
	if(rc == -1){ printf("pipe failed \n"); }

	*pid_1 = fork(); /* command before the pipe*/
	if(*pid_1 == 0){/* CHILD PROCESS FOR pipe write end*/

		close(p[0]); /* close read */
		/* redirect stdout to p[1]*/
		close(1); /* close stdout */
		dup2(p[1], 1); /* stdout is using the address of fd p[1]*/
		close(p[1]); /* after the writing has finished, close write */

		strcpy(FILE, searchPath(getenv("MYPATH"), argv[0]));/* search $MYPATH */;

		if ( strcmp(FILE, "nan") == 0/* search directory; command not found*/) { perror("Command not found. \n");}

		execv(FILE, argv);  /* new image */
		perror("pipe write error:\n"); /* if exec fail will come to this*/

	}

	*pid_2 = fork(); /* command after the pipe*/
	if(*pid_2 == 0){/* CHILD PROCESS FOR pipe read end*/

		close(p[1]); /* close write */
		close(0); /* close stdin */
		dup2(p[0], 0); /* stdin is using the address of fd p[0]*/
		close(p[0]); /* after read, close read*/
		
		#ifdef DEBUG
			printf("pipe: read (not an error): \n");
			fflush(stdout);
		#endif
		strcpy(FILE, searchPath(getenv("MYPATH"), new_arg[0]));/* search $MYPATH */;

		if ( strcmp(FILE, "nan") == 0/* search directory; command not found*/) { perror("Command not found. \n");}

		new_arg = realloc(new_arg, (new_arg_len + 1) * sizeof(char *));
		new_arg[new_arg_len] = NULL;
		if(strcmp(argv[argv_no-1], "&") == 0){
			new_arg[new_arg_len-1] = NULL;
		}
		execv(FILE, new_arg); /* new image */
		perror("pipe read error:\n"); /* if exec fail will come to this */

	} 

	/* parent process */
	int status1, status2;
	pid_t child_pid_1, child_pid_2;

	close(p[0]); // notice that any process having the copy of pipe could read&write to the buffer
	close(p[1]);

	if(strcmp(argv[argv_no-1], "&") != 0){ /* foreground processing*/
		
		while (1) {

			child_pid_1 = waitpid(*pid_1, &status1, 0);  
			child_pid_2 = waitpid(*pid_2, &status2, 0);

			if ((child_pid_1 != 0) || (child_pid_2 != 0)){

				if(child_pid_1 != *pid_1){

					printf("[Process %d terminated with status %d.]", child_pid_1, status1);
					kill(child_pid_1, SIGINT); /* have to manually kill it, check actual bash behavior */

				}else if(child_pid_2 != *pid_2){

					printf("[Process %d terminated with status %d.]", child_pid_2, status2);
					kill(child_pid_2, SIGINT); /* have to manually kill it, check actual bash behavior */

				}else if((child_pid_1 != 0) && (child_pid_2 != 0)){ /* finish execution and return to main*/

					break;
				}
			}

			sleep(1);
		}

		if ((child_pid_1 == -1) || (child_pid_2 == -1)) {
			printf("waitpid() failed for child process %d, %d \n", child_pid_1, child_pid_2);
		}

	}else{ /* background processing */

		printf("[running background process \"%s\"]\n", new_arg[0]); /* todo: print second cmd name */
		printf("%s$ ", getcwd(NULL, 0)); /* todo, TRY POSIX.1-2001 Standard, check man page*/

	}


	free(FILE);
	free(argv);
	free(new_arg);
	return 0;
}

int exec_(char ** argv, int argv_no, pid_t * pid){
	/*
		param: pid, 
		param: argv, {"ls", "-l"} not null terminated
		param: argv_no

	*/
	/* Execute the common cmd

	*/
	#ifdef DEBUG
	printf("exec_: 0 pos, we are at %s \n", getcwd(NULL, 0)); /* todo, TRY POSIX.1-2001 Standard, check man page*/
	#endif

	char * FILE = calloc(MAX_FILEPATH_LEN, sizeof(char)); 
	*pid = fork();
	if ( *pid == -1 ) { /* child process creation failed*/ 
		perror("fork() failed \n"); 
		return EXIT_FAILURE;
	}

	if(*pid == 0){

		strcpy(FILE, searchPath(getenv("MYPATH"), argv[0]));/* search $MYPATH */;

		if ( strcmp(FILE, "nan") == 0/* search directory; command not found*/) { perror("Command not found. \n");}

		argv = realloc(argv, (argv_no + 1) * sizeof(char *));
		argv[argv_no] = NULL;

		if(strcmp(argv[argv_no-1], "&") == 0){
			argv[argv_no-1] = NULL;
		}
		#ifdef DEBUG
		printf("exec_: we are at %s \n", getcwd(NULL, 0)); /* todo, TRY POSIX.1-2001 Standard, check man page*/
		#endif
		execv(FILE, argv);

	}

	/* parent process */
	if(strcmp(argv[argv_no-1], "&") != 0){ /* foreground processing*/
		#ifdef DEBUG
		printf("exec_: not a background process. \n");
		#endif

		int status;
		pid_t child_pid;

		while (1) {

			child_pid = waitpid(0, &status, WNOHANG);  /* 0: Child process having the same group ID (?) */

			if(child_pid != 0){
				if(child_pid == *pid){
					break;
				}else{
					printf("[Process %d terminated with status %d.]", child_pid, status);
					kill(child_pid, SIGKILL); /* have to manually kill it, check actual bash behavior */
				}
			}
			sleep(1);
		}

		if (child_pid == -1) {
			printf("waitpid() failed for child process %d \n", *pid);
		}
	}else{

		printf("[running background process \"%s\"]\n", argv[0]); /* todo: print second cmd name */
		printf("%s$ ", getcwd(NULL, 0)); /* todo, TRY POSIX.1-2001 Standard, check man page*/

	}

	free(FILE);
	free(argv);
	return 0;

}


