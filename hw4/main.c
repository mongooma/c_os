#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 100
#define MAX_BUFFER_SIZE 1024

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

fd_set readfds;
int client_sockets[ MAX_CLIENTS ]; /* client socket fd list */
int client_socket_index = 0;  /* next free spot */
char ** user_list; // = calloc(MAX_CLIENTS, sizeof(char *));  //shared
int user_index = 0; // shared

struct thread_args{
	int fd; 
	// fd_set readfds; //shared
	// int * client_sockets;  //shared
	// char ** user_list;  //shared
	// int user_index;  //shared
};

void * user_routine(void * worker_args){

	/* is there activity on any of the established connections? 

	args:
	struct

	*/
    // check old (and the added new) connections for data

	int n; 
	char * buffer = calloc(MAX_BUFFER_SIZE, sizeof(char));
	struct thread_args * args = (struct thread_args *) worker_args;

	int fd = args->fd;

	if ( FD_ISSET( fd, &readfds ) )
	{
		
		while(1){
			/* can also use read() and write() */
			n = recv( fd, buffer, MAX_BUFFER_SIZE, 0 );
				/* all  commands  are specified in upper-case and end with a newline ('\n') character*/
				/* transfer bytes of streams */
			    /* we know this recv() call will not block (?) */

			if ( n < 0 )
			{
			  perror( "recv()" );
			}
			else if ( n == 0 )
			{
			  int k;
			  printf( "Client on fd %d closed connection\n", fd );
			  close( fd );

			  /* remove fd from client_sockets[] array: */
			  pthread_mutex_lock(&mutex2);
			  for ( k = 0 ; k < client_socket_index ; k++ )
			  {
			    if ( fd == client_sockets[ k ] )
			    {
			      /* found it -- copy remaining elements over fd */
			      int m;
			      for ( m = k ; m < client_socket_index - 1 ; m++ )
			      {
			        client_sockets[ m ] = client_sockets[ m + 1 ];
			      }
			      client_socket_index--;
			      break;  /* all done */
			    }
			  }
			  pthread_mutex_unlock(&mutex2);
			}
			else /* n > 0; deal with different msgs*/
			{
				buffer[n] = '\0';
				// printf( "Received message from %s: %s\n",
				//         inet_ntoa( (struct in_addr)client.sin_addr ),
				//         buffer );
				if(strncmp(buffer, "LOGIN: ", 7) == 0){
				/* 
					LOGIN: <userid>\n
					check <userid>: a string of alphanumeric characters with a length in the range[4,16] */
				  
				  /* extract userid*/

				  char * userid = calloc(256, sizeof(char)); 
				  strcpy(userid, buffer+7);

				  if((strlen(userid) < 4) || (strlen(userid) > 16)){
				  	perror("Invalid userid\n"); /* keep the TCP open*/
				  }
				/* add user to a user list */
				  pthread_mutex_lock(&mutex1);
				  user_list[user_index++] = calloc(strlen(userid) + 1, sizeof(char));
				  strcpy(user_list[user_index++], userid);
				  pthread_mutex_unlock(&mutex1);
				  continue;
				}else if(strncmp(buffer, "WHO", 3) == 0){
				  printf("OK!\n");
				  for(int i=0; i < user_index; i ++){
				  	printf("%s\n", user_list[i]);
				  }

				}else if(strncmp(buffer, "LOGOUT", 6) == 0){
				  /* erase userid from list*/
				  // printf("OK!\n");
				  /* current TCP connection stay open, wait for the remote side to close connection*/
				   continue;
				}else if(strncmp(buffer, "SEND ", 5) == 0){
				  /* check: SEND <recipient-userid> <msglen>\n<messag
					 active user id;
					 [1, 990]
					 error msgs: 

				  */
				  // printf("OK\n");


				  // char * recipient;
				  // struct sockaddr *recipient_addr;

				  // int sent_len;

				  /* FROM <sender-userid> <msglen> <message>\n */
				  /* ssize_t send(int sockfd, const void *buf, size_t len, int flags);
					 ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr, socklen_t addrlen)
				  */
				  // n = sendto( fd, "FROM <sender-userid> <msglen> <message>\n", 
				  				// sent_len, 0, recipient_addr, len(recipient_addr));
				  continue;
				}else if(1/* broadcast*/){
				  /* BROADCAST <msglen>\n<message>*/
				  continue;

				}else if(1/*share*/){
				  /* SHARE <recipient-userid> <filelen>\n*/
				  continue;

				}else if(1/*others*/){
				  continue; 
				}else{
					perror("invalid\n");
				}
			}
		}
	}
    
    return NULL;
}



int main(int argc, char ** argv ){
	/* check command args */
	if((argc > 2) || (argc < 2)){

		perror("ERROR: *.c <port no.>\n");

	}

	/* ====== */
	user_list = calloc(MAX_CLIENTS, sizeof(char *));  //shared
	pthread_t * tid_l = calloc(MAX_CLIENTS, sizeof(pthread_t));

	/* ====== */

	/* Create the listener socket as TCP socket */
	/*   (use SOCK_DGRAM for UDP)               */
	int sock = socket( PF_INET, SOCK_STREAM, 0 );
	// int sock_1 = socket( PF_INET, SOCK_STREAM, 0 ); //udp
	/* note that PF_INET is protocol family, Internet */

	if ( sock < 0 )
	{
		perror( "socket()" );
		exit( EXIT_FAILURE );
	}

	/* socket structures from /usr/include/sys/socket.h */
	struct sockaddr_in server;
	struct sockaddr_in client;
	server.sin_family = PF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	unsigned short port = atoi(argv[0]);

	/* htons() is host-to-network-short for marshalling */
	/* Internet is "big endian"; Intel is "little endian" */
	server.sin_port = htons( port );
	int len = sizeof( server );

	if ( bind( sock, (struct sockaddr *)&server, len ) < 0 )
	{
		perror( "bind()" );
		exit( EXIT_FAILURE );
	}

	/* UDP */
	// if ( bind( sock, (struct sockaddr *)&server, len ) < 0 )
	// {
	// 	perror( "bind()" );
	// 	exit( EXIT_FAILURE );
	// }

	listen(sock,  MAX_CLIENTS);  /*  32 is the number of waiting clients */
	printf("MAIN: Started server\n");
	printf("MAIN: Listening for TCP connections on port %d\n", port );
	printf("MAIN: Listening for UDP connections on port %d\n", port );

	int fromlen = sizeof( client );
  	int i;

  	/* listen the server 0 socket*/

  	while ( 1 )  /* todo, handle UDP in main thread*/
	{
		#if 1
		struct timeval timeout;
		timeout.tv_sec = 2;
		timeout.tv_usec = 500;  /* 2 seconds AND 500 microseconds */
		#endif

		FD_ZERO( &readfds ); /* FD_ZERO() clears a set. */
		FD_SET( sock, &readfds );   /* listener socket, fd 3; 
		                              FD_SET() and FD_CLR() respectively add and remove 
		                              a given file descriptor from a set.*/
		/* initially, this for loop does nothing; but once we have */
		/*  client connections, we will add each client connection's fd */
		/*   to the readfds (the FD set) */
		/* since the select() below modify the readfds set in-place*/
		for ( i = 0 ; i < client_socket_index ; i++ ) // initial: client_socket_index = 0
		{
		  FD_SET( client_sockets[ i ], &readfds ); /* re-initialized readfds*/
		  // printf( "Set FD_SET to include client socket fd %d\n",
		          // client_sockets[ i ] );
		}


		int ready = select( FD_SETSIZE, &readfds, NULL, NULL, &timeout ); /* readfds is modified*/

		/* ready is the number of ready file descriptors */
		// printf( "select() identified %d descriptor(s) with activity\n", ready );

		/* is there activity on the listener descriptor? */
		if ( FD_ISSET( sock, &readfds ) ) /* FD_ISSET() tests to see if a file  descriptor is part of the set;*/
		{
		  int newsock = accept( sock,
		                        (struct sockaddr *)&client,
		                        (socklen_t *)&fromlen );
		         /* this accept() call we know will not block - in readfds*/
		  printf( "MAIN: Rcvd incoming TCP connection from: %s\n", 
		  	inet_ntoa( (struct in_addr)client.sin_addr) );
		  client_sockets[ client_socket_index++ ] = newsock;

		  // open a thread for the new connection
		  // int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
		//                           void *(*start_routine) (void *), void *arg);
		 /* struct thread_args{

			int fd; 
			fd_set readfds; //shared
			int * client_sockets;  //shared
			char ** user_list;  //shared
			int user_index;  //shared

		}*/

		  struct thread_args * client_args = calloc(1, sizeof(struct thread_args));

		  client_args->fd = newsock;
		  // client_args->readfds = readfds;
		  // client_args->client_sockets = client_sockets;
		  // client_args->user_list = user_list;
		  // client_args->user_index = user_index;

		  int rc = pthread_create(&tid_l[client_socket_index], NULL, user_routine, (void *) &client_args);
		  if (rc < 0){
		  	perror("\n");
		  }

		} // add new client connection
	}

	return EXIT_SUCCESS; /* we never get here */
}



