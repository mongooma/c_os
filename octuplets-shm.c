/* octuplets-shm.c */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/* This constant defines the shared memory segment that
   multiple processes can use to attach to a shared memory
   segment (which is created below) */
#define SHM_SHARED_KEY 8192

/* function prototypes */
int child( int j, int k, char * data, int shmid );
void parent( int children, char * data, int shmid );

int main()
{
  /* create the shared memory segment */
  key_t key = SHM_SHARED_KEY;
  int shmid = shmget( key, 1024, IPC_CREAT | 0660 );

  if ( shmid == -1 )
  {
    perror( "shmget() failed" );
    return EXIT_FAILURE;
  }

  /* attach to the shared memory segment */
  char * data = shmat( shmid, NULL, 0 );

  if ( data == (char *)-1 )
  {
    perror( "shmat() failed" );
    return EXIT_FAILURE;
  }

  /* since we attach to the shared memory segment here
      (i.e., before the fork() call), the child processes
       do not need to call shmat() */


  int j = 0;  /* each child process will work on [j,k] */
  int k = 3;  /* e.g., "ABCD"
                 j  k
                 |  |
                 v  v  */
  strcpy( data, "ABCDEFGHIJKLMNOPQRSTUVWXYZZZZZZZ!!!!" );
   /*     ^^^^
          this points to the shared memory */

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
      int rc = child( j, k, data, shmid );
      exit( rc );       /* be sure to exit the child process */
    }

    j += 4;
    k += 4;
  }

  parent( children, data, shmid );

  return EXIT_SUCCESS;
}



/* each child process
   sleeps for t seconds,
   converts the range data[j] to data[k] to lowercase,
   displays the entire data string,
   then returns t */
int child( int j, int k, char * data, int shmid )
{
  srand( time( NULL ) * getpid() * getpid() );

  int t = 10 + ( rand() % 21 );  /* [10,30] */
  printf( "CHILD %d: I'm gonna nap for %d seconds.\n",
          getpid(), t );
  sleep( t );

  while ( j <= k )
  {
    data[j] = tolower( data[j] );
    j++;
  }

  printf( "CHILD %d: I'm awake! \"%s\"\n", getpid(), data );

  /* child process detaches from the shared memory segment */
  int rc = shmdt( data );

  if ( rc == -1 )
  {
    perror( "shmdt() failed" );
    return EXIT_FAILURE;
  }

  return t;
}


void parent( int children, char * data, int shmid )
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

  /* parent process detaches from the shared memory segment */
  int rc = shmdt( data );

  if ( rc == -1 )
  {
    perror( "shmdt() failed" );
    exit( EXIT_FAILURE );
  }

  /* parent process removes the shared memory segment */
  if ( shmctl( shmid, IPC_RMID, 0 ) == -1 )
  {
    perror( "shmctl() failed" );
    exit( EXIT_FAILURE );
  }
}