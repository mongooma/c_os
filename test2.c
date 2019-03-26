#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

/* gcc ... -lpthread */
/* gcc ... -pthread */
#include <pthread.h>


void * whattodo( void * arg );


int main(){


	int CHILDREN = 2;
	pthread_t tid[CHILDREN];


	for ( int i = 0 ; i < CHILDREN ; i++ )
  	{
 		int * t = malloc( sizeof(int));
 		* t = i;
		int rc = pthread_create( &tid[i], NULL, whattodo, t);

	}

	for (int i = 0 ; i < CHILDREN ; i++ )
  	{
 		unsigned int * x;
    	int rc = pthread_join( tid[i], (void **) &x );
    /* int pthread_join(pthread_t thread, void ** retval);*/   

	}


}

void * whattodo( void * arg )
{

	int t = * (int *)arg;
	free(arg);
	
  	printf( "THREAD %u: I'm here, no. %d\n",
          (unsigned int)pthread_self(), t);


	pthread_exit( &t );

}