#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "functions.h"

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

	int m = atoi(argv[1]);
	int n = atoi(argv[2]);
	int x = 0;

	printf("THREAD %ld: Solving Sonny's knight's tour problem for a %dx%d board\n", pthread_self(), m, n);

	/* sumitty: only print dead end msg having found larger than x coverage*/
	
	if(argc > 3){
		x = atoi(argv[3]);
	}

	char ** board; /* initial board configuration */

	/* init global variables */
	max_square = 0;
	deadends = 0;
	global_tid_l_len = 1;
	/* when a dead end is encountered in a thread, that thread checks the
	variable, updating it if a new maximum is found.*/
	/**********/

	/* init the main thread */
	board = calloc(m, sizeof(char *));
	for(int i = 0; i < m; i++){
		board[i] = calloc(n, sizeof(char));
	}

	move_next_arg args;

	args.board = board;
	args.board_size[0] = m;
	args.board_size[1] = n;
	args.current_pos = calloc(2, sizeof(int));
	args.current_pos[0] = 0;
	args.current_pos[1] = 0;
	args.move = 0;
	args.direction = 2;
	args.x = x;

	move_next((void *) &args);  /* the first position has only one possible move - 2*/

	/* join the child threads; order doesn't really matter */
	for(int i = 0; i < thread_no; i ++ ){
		pthread_join(global_tid_l[i], NULL);
	}

	/**/

	printf("THREAD %ld: Best solution(s) found visit %d squares (out of %d)", pthread_self(), max_square, m * n);

	/* print dead end boards; by row */
	for(int k = 0; k < deadends; k ++){
		for(int i = 0; i < m; i ++){
			if(i == 0){
				printf("THREAD %ld: > ", pthread_self());
			}else{
				printf("THREAD %ld:   ", pthread_self());	
			}
			for(int j = 0; j < n; j ++){
				if(dead_end_boards[k][i][j] == 'S'){
					printf("%c", 'S');
				}else{
					printf("%c", '.');
				}
			}
			printf("\n");
		}
	}

	return EXIT_SUCCESS;

}

