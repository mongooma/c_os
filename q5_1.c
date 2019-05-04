#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>


void * go( void * arg ){
	char * s = calloc( strlen( arg ) + 1, sizeof( char ) );
	strcpy( s, arg );
	s[7] = '\0';
	fprintf( stderr, "%s", s );
	free( s );
	return NULL;
}

int main(){
	pthread_t tid;
	char * q = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	pthread_create( &tid, NULL, go, q );
	fprintf( stderr, "%s", q );
	return EXIT_SUCCESS; // this might be reached before the child thread execute
}