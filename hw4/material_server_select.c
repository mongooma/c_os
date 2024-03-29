/* server-select.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <sys/select.h>      /* <===== */

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100      /* <===== */


int main()
{
  /* ====== */
  fd_set readfds;
  int client_sockets[ MAX_CLIENTS ]; /* client socket fd list */
  int client_socket_index = 0;  /* next free spot */
  /* ====== */

  /* Create the listener socket as TCP socket */
  /*   (use SOCK_DGRAM for UDP)               */
  int sock = socket( PF_INET, SOCK_STREAM, 0 );
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

  unsigned short port = 8128;

  /* htons() is host-to-network-short for marshalling */
  /* Internet is "big endian"; Intel is "little endian" */
  server.sin_port = htons( port );
  int len = sizeof( server );

  if ( bind( sock, (struct sockaddr *)&server, len ) < 0 )
  {
    perror( "bind()" );
    exit( EXIT_FAILURE );
  }

  listen( sock, 5 );  /* 5 is number of waiting clients */
  printf( "Listener bound to port %d\n", port );

  int fromlen = sizeof( client );

  char buffer[ BUFFER_SIZE ];

  int i, n;

  while ( 1 )
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
    printf( "Set FD_SET to include listener fd %d\n", sock );

    /* initially, this for loop does nothing; but once we have */
    /*  client connections, we will add each client connection's fd */
    /*   to the readfds (the FD set) */
    /* since the select() below modify the readfds set in-place*/
    for ( i = 0 ; i < client_socket_index ; i++ ) // initial: client_socket_index = 0
    {
      FD_SET( client_sockets[ i ], &readfds ); /* re-initialized readfds*/
      printf( "Set FD_SET to include client socket fd %d\n",
              client_sockets[ i ] );
    }

#if 0
    /* This is a BLOCKING call, but will block on all readfds */
    int ready = select( FD_SETSIZE, &readfds, NULL, NULL, NULL );
#endif

#if 1
    /*  select() can monitor only file descriptors numbers that are less than FD_S
ETSIZE */
    int ready = select( FD_SETSIZE, &readfds, NULL, NULL, &timeout ); /* readfds is modified*/
    /* int select(int nfds, fd_set *readfds, fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout); 

    On exit, each of the file descriptor sets is modified in place to indicate which file descriptors  actu‐
    ally changed status.  (Thus, if using select() within a loop, the sets must be reinitialized before each
    call.)

    On success, select() and pselect() return the number of file descriptors contained in the three returned
    descriptor  sets  (that is, the total number of bits that are set in readfds, writefds, exceptfds) which
    may be zero if the timeout expires before anything interesting happens.

    */
    /* FD_SETSIZE is a macro? */

    if ( ready == 0 )
    {
      printf( "No activity (yet)...\n" );
      continue;
    }
#endif

    /* ready is the number of ready file descriptors */
    printf( "select() identified %d descriptor(s) with activity\n", ready );


    /* is there activity on the listener descriptor? */
    if ( FD_ISSET( sock, &readfds ) ) /* FD_ISSET() tests to see if a file  descriptor
       is part of the set;*/
    {
      int newsock = accept( sock,
                            (struct sockaddr *)&client,
                            (socklen_t *)&fromlen );
             /* this accept() call we know will not block - in readfds*/
      printf( "Accepted client connection\n" );
      client_sockets[ client_socket_index++ ] = newsock;
    } // add new client connection


    /* is there activity on any of the established connections? */
    // check old (and the added new) connections for data
    for ( i = 0 ; i < client_socket_index ; i++ )
    {
      int fd = client_sockets[ i ];

      if ( FD_ISSET( fd, &readfds ) )
      {
        /* can also use read() and write() */
        n = recv( fd, buffer, BUFFER_SIZE - 1, 0 );
            /* we know this recv() call will not block */

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
        }
        else
        {
          buffer[n] = '\0';
          printf( "Received message from %s: %s\n",
                  inet_ntoa( (struct in_addr)client.sin_addr ),
                  buffer );

          /* send ack message back to client */
          n = send( fd, "ACK\n", 4, 0 ); // 
          if ( n != 4 )
          {
            perror( "send() failed" );
          }
        }
      }
    }
  }

  return EXIT_SUCCESS; /* we never get here */
}