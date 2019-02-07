#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h> 
#include <string.h>
#include <ctype.h>
#include <unistd.h> //for unix
#include <errno.h>


extern int errno;

/* doing unit test for source.c*/


char * searchPath(const char * PATH, const char * cmd) {
	/* not debugged */
	/* return char * p for the full path for the executable cmd 
		NULL if no executable found*/

	/* search in $MYPATH to find the executable FILE*/

#ifdef DEBUG
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

#ifdef DEBUG
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
		int erronum;
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
	return NULL;
}




int main(){


	printf("%s \n", searchPath(getenv("PATH"), "sudo"));

	return 0;


}