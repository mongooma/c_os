#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "functions.h"


/* global variables */
int max_square;
int deadends;
pthread_t * global_tid_l;
int global_tid_l_len;
int thread_no;
char *** dead_end_boards;


int main(int argc, char ** argv){

	/* get m * n from cmd arguments
	 ./a.out 3  3  x --> x: display dead end boards with at least x squares covered




	 					 +1

         +0

			     *
			
		     	        +2		
           
            +3	

	 */

	int m = argv[1];
	int n = argv[2];

	if(argc > 3){
		int x = argv[3];
	}

	char ** board; /* initial board configuration */

	int * start = [0, 0];

	/* init global variables */
	max_square = 0;
	deadends = 0;
	/* when a dead end is encountered in a thread, that thread checks the
	variable, updating it if a new maximum is found.*/
	/**********/

	/* init the main thread */
	board = calloc(m, sizeof(char *))
	for(int i = 0; i < m; i++){
		board[i] = calloc(n, sizeof(char));
	}

	move_next(board, [m, n], [0, 0], 2);  /* the first position has only one possible move - 2*/

	/* join the child threads; order doesn't really matter */
	for(int i = 0; i < thread_no; i ++ ){
		pthread_join(global_tid_l[i], NULL);
	}

	/**/

	return EXIT_SUCCESS;

}

