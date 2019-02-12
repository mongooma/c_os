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


argv_data getCmd(const char * buffer, argv_data d) {
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

	#ifdef DEBUG_pass
	for(int i = 0; i < k+1; i ++){

		//printf("%s len: %d\n", argv_user[i], (int) strlen(argv_user[i]));
		printf("%s len: %d\n", d.argv_user[0], (int) strlen(d.argv_user[0]));
	}
	#endif

	d.argv_no[0] = (int)(k+1);

	return d; // actual number of argv
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
	#ifdef DEBUG_pass
		printf("debug:cmd not found \n");
		fflush(stdout);
	#endif
	return "nan";
}



int execute_cmd(char ** argv, const int argv_no, const char * cwd){
	/* to debug */

	/* At parent process */

	/* 4.2.1 special case cd, exit*/
	#ifdef DEBUG_pass
		printf("in execute_cmd: argv[0] %s \n", argv[0]); // couldn't read argv when cmd is "ps -ef | grep", 
		printf("strcmp argv[0], 'cd', %d \n", strcmp(argv[0], "cd"));


	#endif

	if (strcmp(argv[0], "cd") == 0 /* todo: check string comparison*/) {

		/* change directory in the parent process*/

		if (argv_no < 2) { chdir(getenv("HOME")); /* as requested in the hw2 pdf*/}
		if (argv_no > 2) { printf("man cd; cd <dir_name>\n"); }

		int r = chdir(argv[1]); 
		if (r == -1){perror("Error returned by perror: ");}

		return 0;

	}

	if (strcmp(argv[0], "exit") == 0 ){

		/* exit parent process*/

		if (argv_no > 2) {perror("man exit\n");}
		printf("bye\n");
		exit(0); /* terminate the whole process*/

	}

	/* 4.2.2 general case cmd parse */

	char ** new_arg = calloc(argv_no, sizeof(char *));
	char * FILE = calloc(MAX_FILEPATH_LEN, sizeof(char)); 
	int new_arg_len = 0;

	int parsePipe(){
		/* internal function, with side-effect:
			argv,
			new_arg,
			new_arg_len,

			*/
		/* if with pipe: argvs for each process are appended with NULL; NULL has replaced | in the argv */
		/* if pipe, return a new second cmd, with the original argv store the first cmd;
			if not pipe, return argv */

		int p = 0;
		int i = 0;
		int j = 0;

		while(i < argv_no){
			int rc = strcmp(argv[i], "|");
			if (rc == 0 ){ /* detect '|'*/
				p = 1;
				argv[i] = NULL;  /* NULL could stop execvp from parsing the rest cmd*/
				if (i == argv_no - 1){perror("Nothing after pipe: \n"); return -1;}
				i++;

			}
	
			#ifdef DEBUG_pass
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
		new_arg[j] = NULL;
		new_arg_len = j;

		return p;

	}

	int p = parsePipe();

	if(p == -1) return 0; /* pipe cmd abnormal*/

	#ifdef DEBUG_pass
		printf("with pipe: %d \n", p);
	#endif

	if (p == 1){

		/* if with one pipe
			
		    parent (wait)
			 /  \
			/	 \
		child_1	 child_2
		(stdout --> stdin)

		*/

		int p[2];
		int rc = pipe(p);
		if(rc == -1){printf("pipe failed \n");}

		pid_t pid_1 = fork(); /* command before the pipe*/
		if(pid_1 == 0){/* CHILD PROCESS FOR pipe write end*/

			close(p[0]); /* close read */
			/* redirect stdout to p[1]*/
			close(1); /* close stdout */
			dup2(p[1], 1); /* stdout is using the address of fd p[1]*/
			close(p[1]); /* after the writing has finished, close write */

			strcpy(FILE, searchPath(getenv("MYPATH"), argv[0]));/* search $MYPATH */;
			chdir(cwd); /* this is to deal with the side-effect of using searchPath() */

			if ( strcmp(FILE, "nan") == 0/* search directory; command not found*/) { perror("Command not found. \n");}

			execvp(FILE, argv);
			perror("pipe write error:\n"); /* if exec fail will come to this*/

		}

		pid_t pid_2 = fork(); /* command after the pipe*/
		if(pid_2 == 0){/* CHILD PROCESS FOR pipe read end*/

			close(p[1]); /* close write */
			close(0); /* close stdin */
			dup2(p[0], 0); /* stdin is using the address of fd p[0]*/
			close(p[0]); /* after read, close read*/
			
			#ifdef DEBUG_pass
				printf("pipe: read: \n");
				fflush(stdout);
			#endif
			strcpy(FILE, searchPath(getenv("MYPATH"), new_arg[0]));/* search $MYPATH */;
			chdir(cwd); /* this is to deal with the side-effect of using searchPath() */

			if ( strcmp(FILE, "nan") == 0/* search directory; command not found*/) { perror("Command not found. \n");}

			/* take care of the end '&' symbol */
			if(strcmp(argv[argv_no-1], "&") != 0){ /* notice that we did't append NULL to the end of argv*/
				#ifdef DEBUG
					for(int i = 9; i < new_arg_len; i++){
						printf("%s \n", new_arg[i]);
					}

				#endif
				execvp(FILE, new_arg);

			}else{
				new_arg[new_arg_len-1] = NULL;
				execvp(FILE, argv);	
			}

			perror("pipe read error:\n"); /* if exec fail will come to this */

		} else { /* PARENT PROCESS*/

			#ifdef DEBUG_pass
				printf("pipe: parent: \n");
			#endif
			if(strcmp(argv[argv_no-1], "&") != 0){ /* only check if the second cmd has terminated & any background process has terminated*/

				int status;
				pid_t child_pid;

				while (1) {
					child_pid = waitpid(0, &status, WNOHANG);  /* 0: Child process having the same group ID (?) */

					if ((child_pid != -1) && (child_pid != pid_2) && (child_pid != pid_1) && (child_pid != 0)){ /* some background process has terminated */
						printf("[process %d terminated with exit status %d]", child_pid, status); 
					}
					
					if ((child_pid == -1) | (child_pid == pid_2)) break;
					sleep(1);
				}

				if (child_pid == -1) {
					printf("waitpid() failed for child process %d \n", pid_2);
				}
				free(new_arg);
				free(FILE);
				return 0; 

			}else{

				printf("[running background process \"%s\"]\n", argv[0]); /* todo: whose pid to print? */
				
				free(new_arg);
				free(FILE);
				return 0; 

			}

			close(p[0]);
			close(p[1]);
		}

	} else { /* if no pipe */

			argv = realloc(argv, (argv_no + 1) * sizeof(char *)); /* if no pipe, parsePipe is not executed, we need to append a NULL to argv*/
			argv[argv_no] = NULL;

			pid_t pid = fork(); /* each time execute one command*/
		
			if ( pid == -1 ) { /* child process creation failed*/ 
				perror("fork() failed \n"); 
				return EXIT_FAILURE;
			}
			else if( pid == 0){/*CHILD PROCESS*/
				
				strcpy(FILE, searchPath(getenv("MYPATH"), argv[0]));/* search $MYPATH */;
				chdir(cwd); /* this is to deal with the side-effect of using searchPath() */

			#ifdef DEBUG_pass
			printf("FILE: %s \n", FILE);
			fflush(stdout);
			#endif
				if ( strcmp(FILE, "nan") == 0/* search directory; command not found*/) { perror("Command not found. \n");}

				/* deal with the '&' symbol in the end */
				if ( strcmp(argv[argv_no-1], "&") != 0  /*foreground processing*/) {
					execvp(FILE, argv);
				} else{
					argv[argv_no-1] = NULL;
					execvp(FILE, argv);	
				}

			}
			else {/*PARENT PROCESS*/ 
				
				int status;
				pid_t child_pid;


				if ( strcmp(argv[argv_no-1], "&") != 0  /*foreground processing*/) {
					while (1) { /* also check if any background process has terminated */
						child_pid = waitpid(0, &status, WNOHANG); /* wait in the parent process for the child process to change status*/
						if ((child_pid != pid) && (child_pid != -1) && (child_pid != 0)){
							printf("[process %d terminated with status %d] \n", child_pid, status);
							kill(child_pid, SIGKILL);
						}
						if ((child_pid == -1) | (child_pid == pid)) break;/*-1 on error; 0 on not changed; pid on success*/
						sleep(1);
					}

					if (child_pid == -1) {
						printf("waitpid() failed for child process %d \n", pid);
					}

					free(new_arg);
					free(FILE);
					return 0;


				}
				else { /* background processing */
					printf("[running background process \"%s\"]\n", argv[0]); /* check exact output*/
					free(new_arg);
					free(FILE);
					return 0;

				}
			}
	}
	return 0;
}
	
