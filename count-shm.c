/* count-shm.c */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/* This constant defines the shared memory segment such that
   multiple processes can attach to this segment */
#define SHM_SHARED_KEY 8999

int main()
{
  /* create the shared memory segment with a size of 4 bytes */
  key_t key = SHM_SHARED_KEY;
  int shmid = shmget( key, sizeof( int ), IPC_CREAT | /*IPC_EXCL |*/ 0660 );
  printf( "shmget() returned %d\n", shmid );

  if ( shmid == -1 )
  {
    perror( "shmget() failed" );
    return EXIT_FAILURE;
  }

  /* attach to the shared memory segment */
  int * data = shmat( shmid, NULL, 0 );
  if ( data == (int *)-1 )
  {
    perror( "shmat() failed" );
    return EXIT_FAILURE;
  }


  int pid = fork();

  if ( pid > 0 ) usleep( 1);

  /* both parent and child processes will run the code below */

  int i, stop = 1000;

  for ( i = 1 ; i <= stop ; i++ )
  {
    *data += i;    /* ==> *data = *data + i */
  }

  printf( "%s: Sum 1..%d is %d\n",
          pid > 0 ? "PARENT" : "CHILD",
          stop, *data );


  int rc = shmdt( data );

  if ( rc == -1 )
  {
    perror( "shmdt() failed" );
    exit( EXIT_FAILURE );
  }

  if ( pid > 0 )   /* skip this and see what happens */
  {
    wait( NULL );

    /* if don't destroy the shimid identified shared mem segment, the data
    will still accessible if re-run this process, or from other processes
    */

    if ( shmctl( shmid, IPC_RMID, 0 ) == -1 )
    {
      perror( "shmctl() failed" );
      exit( EXIT_FAILURE );
    }
  }

  return EXIT_SUCCESS;
}