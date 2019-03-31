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

typedef struct move_next_args
{
	char ** board; 
	const int * board_size; 
	int * current_pos; 
	int move;
	const int direction;
};


int main(int argc, char ** argv){

	/* get m * n from cmd arguments
	 ./a.out 3  3  x --> x: display dead end boards with at least x squares covered




	 					 +1

         +0

			     *
			
		     	        +2		
           
            +3	

	 */
	setvbuf( stdout, NULL, _IONBF, 0 );

	int m = argv[1];
	int n = argv[2];

	printf("THREAD %d: Solving Sonny's knight's tour problem for a %dx%d board\n", pthread_self(), m, n);

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

	
	struct move_next_args args;

	args.board = board;
	args.board_size = [m, n];
	args.current_pos = [0, 0];
	args.move = 0;
	args.direction = 2;

	move_next((void *) &args);  /* the first position has only one possible move - 2*/

	/* join the child threads; order doesn't really matter */
	for(int i = 0; i < thread_no; i ++ ){
		pthread_join(global_tid_l[i], NULL);
	}

	/**/

	printf("THREAD %d: Best solution(s) found visit %d squares (out of %d)", pthread_self(), max_square, m * n);

	/* print dead end boards; by row */
	for(int k = 0; k < deadends; k ++){
		for(int i = 0; i < m; i ++){
			if(i == 0){
				printf("THREAD %d: > ", pthread_self());
			}else{
				printf("THREAD %d:   ", pthread_self());	
			}
			for(int j = 0; j < n; j ++){
				if(dead_end_boards[k][i][j] == 'S'){
					printf('S');
				}else{
					printf('.');
				}
			}
			printf("\n");
		}
	}

	return EXIT_SUCCESS;

}

