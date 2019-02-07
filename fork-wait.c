nclude <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
	  /* see "man 7 signal" and today's in-class notes for more details */
	  printf( "SIGCHLD is %d\n", SIGCHLD );


	    pid_t pid;   /* process id (pid) */

	      printf( "before fork()...\n" );

	        /* create a new (child) process */
	        pid = fork();

		  if ( pid == -1 )
			    {
				        perror( "fork() failed" );
					    return EXIT_FAILURE;
					      }

		    /* fork() will (attempt to) create a new process by
		     *      duplicating/copying the existing running process */

		    /* both parent and child processes return here */

		    printf( "after fork()...\n" );


		      if ( pid == 0 )
			        {
					    /* delay child process */
					    int i = 0;
					        for ( i = 0 ; i < 1000000000; i++ )
							     ;

						    printf( "CHILD: happy birthday!\n" );

						        return 12;

#if 0
							    char * xyz = NULL;  /* cause a seg-fault: */
							        sprintf( xyz, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" );
#endif
								  }
		        else /* pid > 0 */
				  {
					  /*    sleep( 30 ); */
					      printf( "PARENT: my child process ID is %d\n", pid );

					          /* to have the parent process wait for the child process
						   *         to complete, we can call wait() or waitpid() */

					          printf( "PARENT: waiting for my child process to terminate....\n" );

						      int status;
#if 0
						          pid_t child_pid = wait( &status );   /* wait() BLOCKS indefinitely */
							      pid_t child_pid = waitpid( pid, &status, 0 );  /* block indefinitely */

							          /* block indefinitely and wait on any child process */
							          pid_t child_pid = waitpid( -1, &status, 0 );
#endif

								      pid_t child_pid;

								          while ( 1 )
										      {
											            child_pid = waitpid( -1, &status, WNOHANG );
												          if ( child_pid != 0 ) break;
													        printf( "PARENT: child process not terminated yet...\n" );
														      sleep( 1 );
														          }


									      if ( child_pid == -1 )
										          {
												        perror( "wait() failed" );
													      return EXIT_FAILURE;
													          }

									          printf( "PARENT: child process %d terminated...\n", child_pid );

										      if ( WIFSIGNALED( status ) )   /* child process was terminated due to */
											          {                              /*  a signal (e.g., seg-fault, etc.)   */
													        printf( "...abnormally\n" );
														    }
										          else
												      {
													            int exit_status = WEXITSTATUS( status );
														          printf( "...successfully with exit status %d\n", exit_status );
															      }
											    }

			  printf( "All done\n" );

			    return EXIT_SUCCESS;
}

