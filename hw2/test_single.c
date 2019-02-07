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

	printf("%s \n", getenv("PATH"));

	return 0;

}