/* octuplets-threads-broken.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

#define CHILDREN 8   /* based on a true story.... */

/* function executed by each thread */
void * whattodo( void * arg );

int main()
{
  pthread_t tid[CHILDREN];   /* keep track of the thread IDs (tids) */

  int i, rc;

  int t;  /* statically allocates 4 bytes on the runtime stack */
   /* whoops....each thread is reading this same 4-byte value for t */

  /* create all the child threads */
  for ( i = 0 ; i < CHILDREN ; i++ )
  {
    t = 2 + i * 2;   /* 2, 4, 6, 8, ... */

    printf( "MAIN: Creating a thread to sleep for %d seconds\n", t );

    rc = pthread_create( &tid[i], NULL, whattodo, &t ); // &t might be changed before the thread is executed
                                        /*        ^^
                                           this is always the same memory
                                            address -- whoops!  :-(       */

    if ( rc != 0 )
    {
      fprintf( stderr, "MAIN: Could not create thread (%d)\n", rc );
      return EXIT_FAILURE;
    }
  }

  /* wait for child threads to terminate */
  for ( i = 0 ; i < CHILDREN ; i++ )
  {
    rc = pthread_join( tid[i], NULL );  /* BLOCKING CALL */

    if ( rc != 0 )
    {
      fprintf( stderr, "MAIN: Could not join thread %ld (rc %d)\n", tid[i], rc );
    }
    else
    {
      printf( "MAIN: Successfully joined child thread %ld\n", tid[i] );
    }
  }

  printf( "MAIN: All child threads have terminated\n" );

#if 0
  sleep( 6 );
#endif

  return EXIT_SUCCESS;  /* exit the main thread... */

  /* when the main thread (or any child thread) exits/terminates its process,
     all other threads are terminated immediately */
}

void * whattodo( void * arg )
{
  int t = *(int *)arg; //

  printf( "THREAD %ld: I'm going to nap for %d seconds\n", pthread_self(), t );
  sleep( t );
  printf( "THREAD %ld: I'm awake\n", pthread_self() );

  return NULL;  /* child thread ends here */
}