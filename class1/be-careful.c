/* be-careful.c */

/* Be sure to allocate space for the '\0' character when
    treating data as a printable character string! */

/* Fix the warnings shown when you compile as follows:
 *
 * bash$ gcc -Wall -Werror be-careful.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <string.h>

int main()
{
#if 0
  /* we will go over this on Monday 1/14 */
  char * x = "ABCD";  /* note that *x points to read-only memory */
  x[2] = 'Q';  /* this will seg-fault */
#endif

  char name[6] = "David";      /* "David\0" */
  printf( "hi %s\n", name );   /*       ^^ this is one character/byte */

  /* correct the above (and below) by using name[6] (and xyz[6]) instead */

  name[1] = 'u';
  printf( "hi %s\n", name );

  char xyz[6] = "QRSTU";
  printf( "xyz %s\n", xyz);
  printf( "hi again %s\n", name );   /* why does this output "DuvidQRSTU" ?! */ /* \0 is not allocated*/


  char * path = malloc( 20 );
  strcpy( path, "/cs/goldsd/s19/os" );
  printf( "path is %s\n", path );

  char * path2 = malloc( 20 ); /* this is allocated just after the previous*/
  strcpy( path2, "/cs/goldsd/s19/os" );
  printf( "path2 is %s\n", path2 );

  /* the next string is more than the allocated 20 bytes... */
  strcpy( path, "/cs/goldsd/s19/os/blah/blah/blah/meme" );
  printf( "path is %s\n", path ); /* printed as is*/

  printf( "path2 is %s\n", path2 );  /* what does this line output? why?! */

  free( path );
  free( path2 );   /* why does this seg-fault? */ /* the memory allocated to path2 is disturbed*/ 

  return EXIT_SUCCESS;
}