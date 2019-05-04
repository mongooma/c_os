#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(){
	close( 0 );
	close( 1 );
	char * s = "ARE YOU READY FOR THE EXAM?";
	int p[2];
	pipe( p );
	pid_t pid = fork();
	if ( pid == 0 ){
		write( p[1], s, 20 );
		printf( "%s\n", s );}  // 1 closed, -> p[1]
	else /* pid > 0 */{
		wait( NULL );
		char buffer[1024];
		int rc = read( p[0], buffer, 1024 );
		buffer[14] = buffer[18] = buffer[rc] = '\0';
		fprintf( stderr, "%s%sNOT!\n", buffer + 8, buffer + 15 );
		fprintf( stderr, "%s\n", buffer + 28 ); // nothing?
	}
	return EXIT_SUCCESS;
}