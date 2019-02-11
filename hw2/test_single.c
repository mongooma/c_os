/* test system funcs*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h> 
#include <string.h>
#include <ctype.h>
#include <unistd.h> //for unix


int main(){

	char * argv[] ={"ls", "|", "all"}; 
	int i = strcmp(argv[1], "|");
	printf("i: %d\n", i); 
	//execlp( "/bin/ls",   "ls",    "-l",    "owefijweofjiwe", NULL );

	return 0;

}