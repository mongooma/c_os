#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h> 
#include <string.h>
#include <ctype.h>
#include <unistd.h> //for unix
#include <errno.h>
#include <sys/wait.h>


int prep1(){

	int rc; 
	printf( "ONE\n" ); // p: one

	rc = fork(); 
	printf( "TWO\n" ); // p: two, c: two


	if ( rc == 0 ) { printf( "THREE\n" ); }  // c: three
	if ( rc > 0 ) { printf( "FOUR\n" ); }  // p: four


	return EXIT_SUCCESS;


	/*
	p        c
	----------
	one

	(		 (
	TWO      TWO
	FOUR     THREE
	)		 )


	*/

}

int prep2(){
	int x = 150; 
	printf( "PARENT: x is %d\n", x ); // p: x=150
	
	printf( "PARENT: forking...\n" ); 
	pid_t pid = fork(); 
	printf( "PARENT: forked...\n" ); // p, c

	if ( pid == 0 ) { 
		printf( "CHILD: happy birthday\n" ); 
		x *= 2; // c: x = 300
		printf( "CHILD: %d\n", x ); } 
	else { 
		wait( NULL ); // waitpid(-1, NULL, 0) wait block until a child terminates/changes state
		printf( "PARENT: child completed\n" ); 
		x *= 2; // p: x = 300
		printf( "PARENT: %d\n", x );
	}
	return EXIT_SUCCESS;

	/*

	p 			c 
	-------------
	...
	...			...
				...
	(blocked by wait)
	...

	*/

}

int prep3(){ /*important*/
	/* using malloc will generates the same result; but technically unpredicatable*/

	char * a = "POLYTECHNIC"; 
	char * b = a;
	char * c = calloc( 100, sizeof( char ) ); // initiated by \0 
	// char * c = malloc( 100); // not initiated
	
	printf( "[%s][%s][%s]\n", a + 10, b + 9, c + 8 ); // [C][IC][]  c+ 8 -> (null string) 

	// char ** d = malloc( 100 * 8 ); 
	char ** d = calloc( 100, sizeof( char * ) ); 
	// d[7] = malloc( 20 );
	d[7] = calloc( 20, sizeof( char ) );
	d[6] = c; // **************now d[6] and c share the same address!****************

	strcpy( d[7], b + 5 ); // ECHNIC
	strcpy( d[6], b + 4 ); // TECHNIC c <-> b + 4

	printf( "[%s][%s][%s]\n", d[7], d[6], c + 5 ); // c+5 -> (null)? no: c <-> b + 4
	
	float e = 2.71828; 

	float * f = calloc( 1, sizeof( float ) );  // initiated by 0
	// float * f = malloc( 1 * sizeof( float ));  // initiated by 0
	float * g = f;

	float * h = &e;

	printf( "[%3.2f][%2.2f][%2.1f]\n", *f, *g, *h ); // 0.00, 0.00, 2.7; no prefix 0s
	

	//
	free(c); // necessary?
	free(d);
	free(d[7]); //
	free(f);
	//free(g);

	return EXIT_SUCCESS;

}

int prep4(){ /* revisit*/

	printf( "ONE\n" );  //y
	fprintf( stderr, "ERROR: ONE\n" ); //flushed; stderr is not buffered
	int rc = close( 2 ); // close stderr; return 0
	printf( "==> %d\n", rc );// 0 

	printf( "TWO\n" ); //y
	fprintf( stderr, "ERROR: TWO\n" ); //stderr closed 
	rc = dup2( 1, 2 ); // return value: the new fd; dup(int oldfd, int newfd)
	printf( "==> %d\n", rc ); //

	printf( "THREE\n" ); //y
	fprintf( stderr, "ERROR: THREE\n" ); // y, stderr closed, use stdout 
	
	return EXIT_SUCCESS; 


}

int prep5(){  /* revisit*/

	int fd; 
	close( 2 ); 
	printf( "HI\n" ); 

// #if 0 
	close( 1 ); /* <== add this line later.... */ 
// #endif
	
	fd = open( "output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600 ); //2 is closed, fd takes 2
	
	dup2(fd, 2); // stderr will be using fd

	printf( "==> %d\n", fd ); 
	printf( "WHAT?\n" );     // will be printed to fd since now fd == 1 stdout
	fprintf( stderr, "ERROR\n" ); 	
	
	close( fd );

	/* Add code to redirect all output on stdout and stderr to the output ﬁle.*/





	return EXIT_SUCCESS;

	/*

	(commented out close(1))
	HI
	==> 2
	WHAT?

	(not commented out close(1), no dup2 added)
	HI
	

	(not commented out close(1), with dup2 added)
	terminal:
	HI

	output.txt:
	==> 1
	WHAT?
	ERROR

	*/

}

int prep6(){

	int fd; 
	close( 2 ); 
	printf( "HI\n" );
#if 0 
	close( 1 ); /* <== add this line later.... */ 
#endif

	fd = open( "output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600 );
	printf( "==> %d\n", fd );
	printf( "WHAT?\n" );
	fprintf( stderr, "ERROR\n" );

	int rc = fork();
	if ( rc == 0 ) { 
		printf( "AGAIN?\n" ); 
		fprintf( stderr, "ERROR ERROR\n" );
	} else {
		wait( NULL ); 
	}
	/* both p and c will come here*/

	printf( "BYE\n" ); 
	fprintf( stderr, "HELLO\n" ); 
	close( fd ); 
	return EXIT_SUCCESS;

	/**
	(with close(1) commented out)

	p 				c
	-----------------
	HI
	==> 2
	WHAT?

	AGAIN?
	BYE
				   BYE

	(with close(1) not commented out)
	
	HI


	*/

}

int prep7(){ /* revisit*/

	int rc; 
	int p[2]; 
	rc = pipe( p ); // 0 on success; -1 on error
	printf( "%d %d %d\n", getpid(), p[0], p[1] ); 

	rc = fork();
	if ( rc == 0 ) { 
		rc = write( p[1], "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26 ); // rc changed to 26
	}
	/****notice! no else{} here!***/
	/****parent and child here****/
	if ( rc > 0 ) { 
		char buffer[70]; 
		rc = read( p[0], buffer, 8 ); // no. of bytes returned will be returned
		buffer[rc] = '\0'; 
		printf( "%d %s\n", getpid(), buffer ); 
	}

	printf( "BYE\n" );
	return EXIT_SUCCESS;

	/*
	p                			c
	-----------------------------
	<pid parent> 3 4

	<pid parent> IJKLMNOP 		<pid child> ABCDEFGH   // result: child will always read the first 8 bytes
													   // process communication overhead 
								BYE
	BYE

	*/

}


int fd_renumber(){

	close(1);
	int rc;
	int p[2];
	rc = pipe(p);

	fprintf(stderr, "%d, %d\n", (int) p[0], (int) p[1]); /*1, 3*/


	return 0;


}


int fd_redirect(){

	close(1);
	dup2(2, 1); 
	fprintf(stderr, "HELLO there. \n");

	return 0;

}


int main(){

	fd_redirect();


}