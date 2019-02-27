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

char * get_env_p(char * env_v){
		 
		if(getenv(env_v) == NULL){
			return "/bin#.";
		}else{
			return getenv(env_v);
		}

	return NULL;

}


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
	getcwd(cwd, MAX_PATH_LEN);

	chdir(p); // somehow we need to be under this path

	DIR * dir = opendir(p);   /* open the directory specified by p*/
	if (dir == NULL)
	{
		fprintf(stderr, "ERROR: opendir() failed");
		//kill(getpid(), SIGINT);
		exit(42);
	}
	/* DIR is a directory stream for reading the directory */
	#ifdef DEBUG_mute
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
      		perror("ERROR: ");
			continue; /* ignore some broken file*/
		}

		if (strcmp(file->d_name, s) == 0) 
		{
			if (S_IXUSR & (buf.st_mode == 0)) { /* S_IXUSR: macro mask value; 
											if the owner has exec permission										*/
				printf("No exec permission \n");
				chdir(cwd);
				free(cwd);
				return 1;
			}else{
				chdir(cwd);
				free(cwd);
	#ifdef DEBUG
	printf("searchSinglePath: we are at %s \n", getcwd(NULL, 0)); /* todo, TRY POSIX.1-2001 Standard, check man page*/
	#endif
				return 0;
			}
		}

	}
	/* if nothing related is found*/

	closedir(dir);
	chdir(cwd);
	free(cwd);

	return -1;
}

char * searchPath(const char * PATH, const char * cmd) {
	/* debugged */
	/* return char * p for the full path for the executable cmd 
		NULL if no executable found*/

	/* search in $MYPATH to find the executable FILE*/

	#ifdef DEBUG_mute
		FILE * f = fopen("./log.txt", "w");
		fprintf(f, "PATH: %s \n", PATH);
	#endif //debug
	
	char * p = calloc(MAX_PATH_LEN, sizeof(char));
	int i = 0;
	int j = 0;
	int FOUND = -1;

	char * PATH_endAdd = calloc(strlen(PATH) + 4, sizeof(char)); 
	/* PATH + '#'' + '.' + '#' + '\0' */
	PATH_endAdd = strcpy(PATH_endAdd, PATH);
	PATH_endAdd[strlen(PATH)] = (char) '#';
	PATH_endAdd[strlen(PATH) + 1] = (char)'.';
	PATH_endAdd[strlen(PATH) + 2] = (char)'#';
	PATH_endAdd[strlen(PATH) + 3] = '\0';


	while (PATH_endAdd[i] != '\0') {
		/*todo: assume the maximum length for $MYPATH is 1024*/
		p[j] = PATH_endAdd[i];

		if ((PATH_endAdd[i] == ':') || (PATH_endAdd[i] == '#')) {  /* if a complete path is stored*/
			/* in hw2 pdf the separator is '#'*/
			/* TODO: using PATH[i] == '#' || ':' won't give the right result*/

			p[j] = '\0';
	#ifdef DEBUG
			FILE * f = fopen("./log.txt", "w");
			fprintf(f, "%s\n", p);
	#endif //debug
			
			FOUND = searchSinglePath(p, cmd);
	#ifdef DEBUG
	printf("searchPath: we are at %s \n", getcwd(NULL, 0)); /* todo, TRY POSIX.1-2001 Standard, check man page*/
	#endif


			/*  0 if an executable s is found;
				1 if an not-executable s is found;
				NULL if other errors*/

			if (FOUND == 0) { /* An executable cmd is found under p*/
				p[j] = '/';
				j ++;
				while(*cmd != '\0') {p[j] = *cmd; cmd ++; j ++;} /* copy cmd to the end of the p path*/
				p[j] = '\0';

				free(PATH_endAdd);
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
	free(PATH_endAdd);
	free(p);
	#ifdef DEBUG
		printf("debug:cmd not found \n");
		fflush(stdout);
	#endif
	return NULL;
}



int execute_cmd(char ** argv, int argv_no){
	/* 	param: argv, an array of strings, i.e {"ls", "-l"} not null terminated
		param: argv_no, number of argvs
		return: 
	*/
	/* master pipeline fro */
	/* to debug */

	/* ******************At parent process *****************/

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
			perror("ERROR: ");
			return 0;
		}

		return 0;

	}


	/************************/

	/* handling normal kind of cmds */

	char ** new_arg = calloc(argv_no, sizeof(char *));
	int new_arg_len = -1;
	int rc;

	int p = parsePipe(argv_no, argv, new_arg, &new_arg_len);

	if(p == -1) return -1; 

	#ifdef DEBUG
		printf("with pipe: %d \n", p);
	#endif

	if(	p == 1 ){ /* with pipe */
		pid_t pid_1, pid_2;
		#ifdef DEBUG
		printf("execute_cmd: before pipe exec, we are at %s \n", getcwd(NULL, 0)); /* todo, TRY POSIX.1-2001 Standard, check man page*/
		#endif
		rc = exec_pipe_2(argv, argv_no, new_arg, new_arg_len, &pid_1, &pid_2); // special pipe case for hw2 
		/* will return in parent process*/
		#ifdef DEBUG
		printf("pid_1: %d, pid_2: %d, here \n", pid_1, pid_2);
		#endif

		for(int i = 0; i < new_arg_len; i ++){
			free(new_arg[i]);
		}

	}else{ /*without pipe */
		pid_t pid;
		#ifdef DEBUG
		printf("execute_cmd: before exec, we are at %s \n", getcwd(NULL, 0)); /* todo, TRY POSIX.1-2001 Standard, check man page*/
		#endif
		rc = exec_(argv, argv_no, &pid, NULL, -1);
		/* will return in parent process*/;

		#ifdef DEBUG
		printf("pid: %d, here \n", pid);
		#endif

	}

	/* both background/foreground will come here */

	free(new_arg);
	return rc;

}

int parsePipe(int argv_no, char ** argv, char ** new_argv, int * new_arg_len){
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
			if (i == argv_no - 1){fprintf(stderr, "ERROR: pipe empty second argv \n"); return -1;}
			i++; // hop over the '|'

		}

		#ifdef DEBUG
		printf("parsePipe: %s \n", argv[i]); 
		fflush(stdout);
		#endif

		if (p == 1){ /* start copy the second argv to new_arg*/
			new_argv[j] = calloc(MAX_ARGV_LEN, sizeof(char));
			new_argv[j] = strcpy(new_argv[j], argv[i]);
			j++;
		}

		i++;
	}

	*new_arg_len = j;

	return p;

}


int exec_pipe_2(char ** argv, int argv_no, char ** new_argv, int new_argv_len, pid_t * pid_1, pid_t * pid_2){
	/* to debug */

	/* 	param: pid_1 pid of the first child process; place holder
		param: pid_2 pid of the second child process; place holder
		param: argv, in the format {"ls", "-l", NULL, "wc", "-l"} anything after NULL will be discarded
		param: argv_no, in the case {"ls", "-l", NULL, "wc", "-l"} argv_no == 5
		param: new_arg, in the case {"ls", "-l", NULL, "wc", "-l"} will be {wc, -l}, not NULL terminated
		param: new_arg_no, in the case {"ls", "-l", NULL, "wc", "-l"} will be 2
		return: 

	*/
	

	/* if with one pipe
		
	    parent 
		 /  \
		/	 \
	child_1 parent (wait for 1)

			 /  \
			/	 \
		child_2  parent (wait for 2)


	(stdout --> stdin)

	*/

	/* this pipe fd place holder will be copied to child processes*/
	int p[2];
	int rc = pipe(p);
	if(rc == -1){ printf("pipe failed \n");}
	exec_(argv, argv_no, pid_1, p, 0);

	exec_(new_argv, new_argv_len, pid_2, p, 1);

	return 0;

}


int exec_(char ** argv, int argv_no, pid_t * pid, int * p, int pipe_pos){
	/*
		Generalized for pipe

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
		printf("fork() failed \n"); 
		//kill(getpid(), SIGINT);
		exit(42);
	}

	if(*pid == 0){
	
		if(p != NULL){ /* if to use pipe*/

			if( pipe_pos == 0){
				/* redirect p[1] write to stdout */
				close(p[0]);
				close(1);
				dup2(p[1], 1);
				close(p[1]);

			}else if( pipe_pos == 1){
				/* redirect p[0] read to stdin */
				close(p[1]);
				close(0);
				dup2(p[0], 0);
				close(p[0]);

			}else{

				perror("exec_: wrong pipe_pos given, check code!"); //error in code

			}
		}

		char * cmd = searchPath(get_env_p("MYPATH"), argv[0]);
		if(cmd != NULL){

			strcpy(FILE, cmd);/* search $MYPATH */;

		}else{

			fprintf(stderr, "ERROR: command \"%s\" not found\n", argv[0]);
			//kill(getpid(), SIGINT); // commit suicide
			exit(42);
			
		}

		argv = realloc(argv, (argv_no + 1) * sizeof(char *));
		argv[argv_no] = NULL;

		if(strcmp(argv[argv_no-1], "&") == 0){
			argv[argv_no-1] = NULL;
		}
		#ifdef DEBUG
		printf("exec_: we are at %s \n", getcwd(NULL, 0)); /* todo, TRY POSIX.1-2001 Standard, check man page*/
		#endif
		/* make a copy of argv and free argv (mem check issue)*/
		//char ** argv_cpy = calloc(MAX_CMD_LEN, sizeof(char *));
		//for(int i = 0; i < argv_no+1; i ++){
		//	argv_cpy[i] = argv[i];
		//} 
		
		//free(argv);

		execv(FILE, argv);

	}

	/* parent process */

	if((p != NULL) && (pipe_pos == 1) ){ /* if close(p[0]) for both argvs in pipe, the second argv will not get input */

		close(p[0]); // close read-in

	}

	if(strcmp(argv[argv_no-1], "&") != 0){ /* foreground processing*/
		#ifdef DEBUG
		printf("exec_: not a background process. \n");
		for(int i = 0; i < argv_no; i ++){
			printf("exec_: %s\n", argv[i]);
		}

		#endif

		int status;
		pid_t child_pid;

		while (1) {

			child_pid = waitpid(0, &status, WNOHANG);  /* 0: Child process having the same group ID (?) */

			if((child_pid != 0) && (child_pid != -1)){
				if(child_pid == *pid){
					break;
				}else{ /* some background process has terminated */
					printf("[Process %d terminated with status %d.]\n", child_pid, status);
					//kill(child_pid, SIGKILL); /* have to manually kill it, check actual bash behavior */
				}
			}
			sleep(1);
		}

		if (child_pid == -1) {
			printf("waitpid() failed for child process %d \n", *pid);
		}

		if(WEXITSTATUS(status) == 42){
			return -1; 
		}

	}else{

		printf("[running background process \"%s\"]\n", argv[0]); /* todo: print second cmd name */
		printf("%s$ ", getcwd(NULL, 0)); /* todo, TRY POSIX.1-2001 Standard, check man page*/

	}

	if(p != NULL){

		close(p[1]); //close write-out

	}
	free(FILE);
	return 0;

}