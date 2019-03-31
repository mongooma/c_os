/* octuplets-threads-variables.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

/* all threads will have access to this global variable */
int x = 5;
  /* this variable should end up being 45, but may sometimes
      be less than 45 due to the lack of synchronization */

/* the mutex variable will be used to guarantee that x is always 45 */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


#define CHILDREN 8   /* based on a true story.... */

/* function executed by each thread */
void * whattodo( void * arg );

int main()
{
  pthread_t tid[CHILDREN];   /* keep track of the thread IDs */
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

  /* wait for child threads to terminate */
  for ( i = 0 ; i < CHILDREN ; i++ )
  {
    unsigned int * x;
    rc = pthread_join( tid[i], (void **)&x );   /* BLOCKING CALL */

    if ( rc != 0 )
    {
      fprintf( stderr, "MAIN: Could not join thread (%d)\n", rc );
    }
    else
    {
      printf( "MAIN: Joined child thread %u, which returned %u\n",
              (unsigned int)tid[i], *x );
      free( x );
    }
  }

  printf( "MAIN: All threads terminated\n" );
  printf( "MAIN: x is %d\n", x );

  return EXIT_SUCCESS;
}

void * whattodo( void * arg )
{
  int t = *(int *)arg;
  free( arg );

  printf( "THREAD %u: I'm going to nap for %d seconds\n",
          (unsigned int)pthread_self(), t );

  /* add 5 to the global variable x, which all threads share */
  x += 5;

#if 0
  /* this code uses the mutex variable to guarantee MUTUAL EXCLUSION */
  pthread_mutex_lock( &mutex );
    /* add 5 to the global variable x, which all threads share */
    x += 5;
  pthread_mutex_unlock( &mutex );
#endif

  sleep( t );
  printf( "THREAD %u: I'm awake\n", (unsigned int)pthread_self() );

  /* dynamically allocate memory to hold the thread's return value */
  unsigned int * y = malloc( sizeof( unsigned int ) );
  *y = pthread_self();
  pthread_exit( y );
}