/* fork-interleaving.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
  pid_t pid;   /* process id (pid) */

  printf( "before fork()...\n" );

  float * f;
  *f = 12.111;  /* uninitialized warning here*/

  /* create a new (child) process */
  pid = fork();

  if ( pid == -1 )
  {
    perror( "fork() failed" );
    return EXIT_FAILURE;
  }

  /* fork() will (attempt to) create a new process by
     duplicating/copying the existing running process */

  /* both parent and child processes return here */

  printf( "after fork()...\n" );

  if ( pid == 0 )
  {
/*    sleep( 30 ); */
    printf( "CHILD: happy birthday!\n" );

  /* delay BOTH parent and child process -- unpredictable
      which one will then be first in the ready queue */
    //int i = 0;
    //for ( i = 0 ; i < 1000000000; i++ )

    execlp("/bin/ls", "any_name_here_will_do", "-1", "shiehjciqba", "NULL"); /*replaced with a new process image*/
    /*will never reach here unless execlp failed (and not the inner command itself failed... try file not exit error) - and terminated and come here*/
    printf("shiahjbcuabjk\n");
    perror("execlp failed \n");
    return EXIT_FAILURE;
   
  }
  else /* pid > 0 */
  {
/*    sleep( 30 ); */
    printf( "PARENT: my child process ID is %d\n", pid );

    /* to have the parent process wait for the child process
        to complete, we can call wait() or waitpid() */

    int status; /* 4 bytes */
    /*  If status is not NULL, wait() and waitpid() store status information in the int
        &status is needed for the address
    .*/
    pid_t child_pid;

    //pid_t child_pid = wait(&status); 
    /* wait(): 
        The wait() system call suspends execution of the calling process until  one  of  its
       children terminates.

        return the pid of the child process who's terminated
    */
    

    while(1){ 
        printf("child process %d not terminated yet\n", pid);
        pid_t child_pid = waitpid(pid, &status, WNOHANG); /*check WNOHANG*/ /*waitpid: */
        /*if WNOHANG was specified and one or more child(ren) specified by pid exist, but have
       not yet changed state, then 0 is returned.*/
        if (child_pid != 0) break;
        sleep(1); 
    }
    //pid_t child_pid = waitpid(-1, &status, 0);   /*wait for any child process*/

    printf("%d terminated, status %d \n", child_pid, WEXITSTATUS(status)); /* todo: check WEIXTSTATUES*/

    printf( "All done\n" );


  }

  return EXIT_SUCCESS;
}