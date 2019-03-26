/* octuplets-threads.c */

/* bash$ gcc -Wall octuplets-threads.c -pthread */

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

  /* create all the child threads */
  for ( i = 0 ; i < CHILDREN ; i++ )
  {
    int * t = malloc( sizeof( int ) );

    *t = 2 + i * 2;   /* 2, 4, 6, 8, ... */

    printf( "MAIN: Creating a thread to sleep for %d seconds\n", *t );

    rc = pthread_create( &tid[i], NULL, whattodo, t );

    if ( rc != 0 )
    {
      fprintf( stderr, "MAIN: Could not create thread (%d)\n", rc );
      return EXIT_FAILURE;
    }
  }

#if 0
  /* wait for child threads to terminate */
  for ( i = CHILDREN - 1 ; i >= 0 ; i-- )
#endif

  /* wait for child threads to terminate */
  for ( i = 0 ; i < CHILDREN ; i++ )
  {
    unsigned int * x;
    rc = pthread_join( tid[i], (void **)&x );  /* wait for the thread to terminate*/ /* BLOCKING CALL */
                            /* ^^^^^^^^^^^
                             change this back to NULL if
                             you do not need the return value */

    if ( rc != 0 )
    {
      fprintf( stderr, "MAIN: Could not join thread %ld (rc %d)\n", tid[i], rc );
    }
    else
    {
      //printf( "MAIN: Successfully joined child thread %ld, which returned %u\n",
       //       tid[i], *x );
      printf( "MAIN: Successfully joined child thread %u, which returned %u\n",
              (unsigned int)tid[i], *x );

      free( x );  /* Whoops, forgot to add this during class! */
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
  int t = *(int *)arg;
  free( arg );

  //printf( "THREAD %ld: I'm going to nap for %d seconds\n", pthread_self(), t );
  printf( "THREAD %u: I'm going to nap for %d seconds\n", (unsigned int)pthread_self(), t );
  sleep( t );
  printf( "THREAD %ld: I'm awake\n", pthread_self() );

#if 0
  return NULL;  /* child thread ends here */
#endif

  /* Use the code below if you want to have your thread code return
      a value back to pthread_join() */

  /* dynamically allocate memory to hold this thread's return value(s) */
  unsigned int * x = malloc( sizeof( unsigned int ) );
  *x = pthread_self();
  pthread_exit( x );
}