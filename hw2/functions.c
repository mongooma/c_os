#include functions.h
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

char * searchPath(const char * PATH, const char * cmd) {
	/* debugged */
	/* return char * p for the full path for the executable cmd 
		NULL if no executable found*/

	/* search in $MYPATH to find the executable FILE*/

	#ifdef DEBUG_pass
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
	/* to debug */

	/* At parent process */

	if (/*pipe*/){

		/* if with one pipe
			
		    parent (wait)
			 /  \
			/	 \
		child_1	 child_2
		(stdout --> stdin)

		*/

		pid_t pid_1 = fork(); /* command before the pipe*/
		pid_t pid_2 = fork(); /* command after the pipe*/
		int p[2];
		int rc = pipe(p);

		if(pid_1 == 0){

			/* redirect stdout to p[1]*/
			dup2(p[1], 1); /* stdout is using the address of fd p[1]*/
			execvp();


		}else{}; // for pipe, parent doesn't wait for the first command to end regardless of '&'

		if(pid_2 == 0){

			dup2(p[0], 0); /* stdin is using the address of fd p[0]*/
			execvp();

		}else{ /* only check if the second cmd has terminated & any background process has terminated*/

			

			if(/* not &*/){

				int status;
				pid_t child_pid;

				while (1) {
					child_pid = waitpid(pid_2, &status, WNOHANG); 
					if (child_pid != 0) break;
					sleep(1);
				}

				if (child_pid == -1) {
					printf("waitpid() failed for child process %d \n", pid);
				}

				if(/* the terminated process is a background process; maintain a pid table for background processes*/){
					printf("[process %d terminated with exit status %d]", pid_1, status); /* todo: child's pid is only visible to parent; another way is to check parent's waitpid after each command*/
				}


			}else{

				printf("[running background process \"%s\"]\n", argv[0]);

			}
		}

	} else {

			pid_t pid = fork(); /* each time execute one command*/
		
			if ( pid == -1 ) { /* child process creation failed*/ 
				perror("fork() failed \n"); 
				return EXIT_FAILURE;
			}
			else if( pid == 0){
				/*CHILD PROCESS*/
				execute_cmd(argv, argv_no);

			}
			else {
				/*PARENT PROCESS*/ 
				int status;
				pid_t child_pid;

				if ( strcmp(argv[argv_no-1], "&") != 0  /*foreground processing*/) {
					while (1) { /* todo: also check if any background process has terminated */
						child_pid = waitpid(pid, &status, WNOHANG); /* wait in the parent process for the child process to finish*/
						if (child_pid != 0) break;/*-1 on error; 0 on not changed; pid on success*/
						sleep(1);
					}

					if (child_pid == -1) {
						printf("waitpid() failed for child process %d \n", pid);
					}

				}
				else { /* background processing */
					printf("[running background process \"%s\"]\n", argv[0]);
				}
			}
	}

}
	
