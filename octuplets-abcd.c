/* octuplets-abcd.c */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

/* global variable */
char data[1024];


/* function prototypes */
int child( int j, int k );
void parent( int children );

int main()
{
  int j = 0;  /* each child process will work on [j,k] */
  int k = 3;  /* e.g., "ABCD"
                 j  k
                 |  |
                 v  v  */
  strcpy( data, "ABCDEFGHIJKLMNOPQRSTUVWXYZZZZZZZ!!!!" );
  
  int i, children = 8;

  for ( i = 0 ; i < children ; i++ )
  {
    pid_t pid = fork();

    if ( pid < 0 )
    {
      perror( "fork() failed" );
      return EXIT_FAILURE;
    }

    if ( pid == 0 )
    {
      int rc = child( j, k );
      exit( rc );       /* be sure to exit the child process */
    }

    j += 4;
    k += 4;
  }

  parent( children );

  return EXIT_SUCCESS;
}



/* each child process converts the range data[j] to data[k]
   to lowercase, sleeps for t seconds, displays the entire
   data string, then returns t */
int child( int j, int k )
{
  while ( j <= k )
  {
    data[j] = tolower( data[j] );
    j++;
  }

  srand( time( NULL ) * getpid() * getpid() ); /* The srand() function sets its argument as the seed for a  new
       sequence  of pseudo-random integers to be returned by rand().*/
      /* time() returns the no. of seconds since "beginning of time" */

  int t = 10 + ( rand() % 21 );  /* [10,30] */ /* rand returns a random int from [0, MAX] */
  printf( "CHILD %d: I'm gonna nap for %d seconds.\n",
          getpid(), t );
  sleep( t );
  printf( "CHILD %d: I'm awake! \"%s\"\n", getpid(), data );
  return t;
}


void parent( int children )
{
  int status;  /* exit status from each child process */

  pid_t child_pid;

  printf( "PARENT: I'm waiting for my children to wake up.\n" );
  printf( "PARENT: \"%s\"\n", data );

  while ( children > 0 )
  {
    /* wait until a child process exits */
    child_pid = wait( &status );   /* BLOCK */

    children--;

    printf( "PARENT: child %d terminated...", child_pid );

    if ( WIFSIGNALED( status ) )
    {
      printf( "abnormally\n" );  /* core dump or kill or kill -9 */
    }
    else if ( WIFEXITED( status ) )
    {
      int rc = WEXITSTATUS( status );
      printf( "successfully with exit status %d\n", rc );
    }

    printf( "PARENT: %d children to go....\n", children );
  }

  printf( "PARENT: All of my children are awake!\n" );
  printf( "PARENT: \"%s\"\n", data );
}