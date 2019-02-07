
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
		
		int x = 42; /* x is statically allocated memory; 4 bytes*/

		printf("size of the int is %ld\n", sizeof( int ));
		printf("x is %d\n", x);

		int * y = NULL; /* statically allocated; hopefully point to a valid address*/

		y = &x;
		printf("y is %d\n", *y);

		printf("size of int* is %ld\n", sizeof( int *)); /*8 bytes; place holder*/
		printf("size of char* is %ld\n", sizeof( char *));


		return EXIT_SUCCESS; /* 0 */
}
