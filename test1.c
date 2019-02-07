/* pipe-ipc.c */

/* TO DO: identify (and draw a diagram that shows) all possible
          outputs of this code.... */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
  int p[2];   /* array to hold the two pipe (file) descriptors:
                 p[0] is the read end of the pipe
                 p[1] is the write end of the pipe */

  int rc = pipe( p );  /* the input argument p will be filled in
                           with the assigned descriptors */

  if ( rc == -1 )
  {
    perror( "pipe() failed" );
    return EXIT_FAILURE;
  }

  /* fd table:

     0: stdin
     1: stdout
     2: stderr                   +--------+
     3: p[0] <======READ======== | buffer | think of this buffer as a
     4: p[1] =======WRITE======> | buffer |  temporary hidden transient file
                                 +--------+
   */

  printf( "Created pipe; p[0] is %d and p[1] is %d\n", p[0], p[1] );


  pid_t pid = fork();   /* fork() will copy the fd table to the child */

  if ( pid == -1 )
  {
    perror( "fork() failed" );
    return EXIT_FAILURE;
  }

  /* fd table:     [each process has its own fd table]

     [PARENT]                                          [CHILD]
     0: stdin                                          0: stdin
     1: stdout                                         1: stdout
     2: stderr                   +--------+            2: stderr
     3: p[0] <======READ======== | buffer | ===READ==> 3: p[0]
     4: p[1] =======WRITE======> | buffer | <==WRITE== 4: p[1]
                                 +--------+
   */

  /* fd table:     [each process has its own fd table]

     *** after we call close() in child and parent below ***

     [PARENT]                                          [CHILD]
     0: stdin                                          0: stdin
     1: stdout                                         1: stdout
     2: stderr                   +--------+            2: stderr
     3: p[0] <======READ======== | buffer |            3:
     4:                          | buffer | <==WRITE== 4: p[1]
                                 +--------+
   */

  if ( pid == 0 )  /* CHILD */
  {
    close( p[0] );  /* close the read end of the pipe in the child process */
    sleep(10);

    /* write data to the pipe */
    printf("We are here!\n"); /*try this. even the parent terminated early this will still run as a zombie*/

    close( p[1] );
  }
  else  /* PARENT */
  {
    close( p[1] );  /* close the write end of the pipe in the parent process */

    char *e=NULL;
    sprintf(e, "shienchi");

    int bytes_written = write( p[1], "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26 );
    printf( "CHILD: Wrote %d bytes to the pipe\n", bytes_written );

    /* read data from the pipe */
    char buffer[80]; /* the read() call below blocks until data is there */
    int bytes_read = read( p[0], buffer, 10 );
    buffer[bytes_read] = '\0';  /* assume the data is string data */
    printf( "PARENT: Read %d bytes: \"%s\"\n", bytes_read, buffer );

    /* read data from the pipe */
    bytes_read = read( p[0], buffer, 10 );
    buffer[bytes_read] = '\0';  /* assume the data is string data */
    printf( "PARENT: Read %d bytes: \"%s\"\n", bytes_read, buffer );

    /* read data from the pipe */
    bytes_read = read( p[0], buffer, 10 );
    buffer[bytes_read] = '\0';  /* assume the data is string data */
    printf( "PARENT: Read %d bytes: \"%s\"\n", bytes_read, buffer );

    /* read data from the pipe */
    bytes_read = read( p[0], buffer, 10 );
    buffer[bytes_read] = '\0';  /* assume the data is string data */
    printf( "PARENT: Read %d bytes: \"%s\"\n", bytes_read, buffer );

    close( p[0] );
  }

  /* fd table:     [each process has its own fd table]

          *** after the child process terminates ***

     [PARENT]
     0: stdin
     1: stdout
     2: stderr                   +--------+
     3: p[0] <======READ======== | buffer |  read() will return 0 if the
     4:                          | buffer |   pipe has no active writers
                                 +--------+
   */

  return EXIT_SUCCESS;
}