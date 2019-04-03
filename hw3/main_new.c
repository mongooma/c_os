#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "functions_new.h"

int MAX_DEAD_ENDS=1000; // work_around; 10000 will cause stack overflow 
int MAX_TIDS_NO=1000;

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

	// Validate the inputs m and n to be sure both are integers greater than 2.  
	// Further, if present, validate input x to be sure it is a positive integer no greater than m×n.  
	// If invalid, display the followingerror message tostderr:

		/* sumitty: only print dead end msg having found larger than x coverage*/
	
	int m, n;
	int x = 0;

	if(argc > 3){
		m = atoi(argv[1]);
		n = atoi(argv[2]);
		x = atoi(argv[3]);

		if( x > (m * n)){
			fprintf(stderr, "ERROR: Invalid argument(s)\n"
							"USAGE: a.out <m> <n> [<x>]\n");

			return EXIT_FAILURE;
		}

	}else if(argc == 3){
		m = atoi(argv[1]);
		n = atoi(argv[2]);

		if((m <= 2) || (n <= 2)){
			fprintf(stderr, "ERROR: Invalid argument(s)\n"
							"USAGE: a.out <m> <n> [<x>]\n");

			return EXIT_FAILURE;
		}
	}else{
		fprintf(stderr, "ERROR: Invalid argument(s)\n"
						"USAGE: a.out <m> <n> [<x>]\n");

		return EXIT_FAILURE;

	}

		


	printf("THREAD %ld: Solving Sonny's knight's tour problem for a %dx%d board\n", pthread_self(), m, n);



	char ** board; /* initial board configuration */

	/* init global variables */
	max_square = 0;
	deadends = 0;
	// global_tid_l = calloc(MAX_TIDS_NO, sizeof(pthread_t)); // realloc failed; workaround
	global_tid_l = calloc(1, sizeof(pthread_t)); // realloc failed; workaround
	global_tid_l_len = MAX_TIDS_NO;
	thread_no = 0;

	dead_end_boards = calloc(MAX_DEAD_ENDS, sizeof(char **)); // realloc failed; workaround
	dead_end_cov = calloc(MAX_DEAD_ENDS, sizeof(int));
	for(int i = 0; i < MAX_DEAD_ENDS; i ++){
		dead_end_boards[i] = calloc(m, sizeof(char *));
		for(int j = 0; j < m; j ++){
			dead_end_boards[i][j] = calloc(n, sizeof(char));
		}

	}

	/* when a dead end is encountered in a thread, that thread checks the
	variable, updating it if a new maximum is found.*/
	/**********/

	/* init the main thread */
	board = calloc(m, sizeof(char *));
	for(int i = 0; i < m; i++){
		board[i] = calloc(n, sizeof(char));
	}

	#ifdef DEBUG_pass
		printf("main: here1 \n");
	#endif

	move_next_arg args;

	args.board = board;
	args.board[0][0] = 'S';
	args.board_size[0] = m;
	args.board_size[1] = n;
	args.current_pos = calloc(2, sizeof(int));
	args.current_pos[0] = 0;
	args.current_pos[1] = 0;
	args.move = 1;
	args.direction = 6;
	args.x = x;
	args.parent_tid = pthread_self();

	#ifdef DEBUG_pass
		printf("main: here2 \n");
	#endif

	move((void *) &args);  /* the first position has only one possible move - 2*/


	/* join the child threads; order doesn't really matter */
	#ifndef NO_PARALLEL
	unsigned int max_covered;
	for(int i = 0; i < thread_no; i ++ ){
	// 	unsigned int * covered;

	// 	// debug: 3 * 4 can't reach i > 15; global_tid_l[i] is okay though
		pthread_join(global_tid_l[i], (void **)&max_covered); // block
		//pthread_join(global_tid_l[i], NULL); // block
		fprintf(stderr, "%ld\n", global_tid_l[i]);
	// 	if(*covered <= (m * n)){
	// 		if(*covered > *max_covered){
	// 			*max_covered = *covered;
	// 		}
	// 		if(*covered > args.x){
	// 			printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", pthread_self(), 
	// 												global_tid_l[i], *covered);
	// 		}
	// 	}
	}
	#endif


	/**/

	//sleep(20);	
	printf("THREAD %ld: Best solution(s) found visit %d squares (out of %d)\n", pthread_self(), max_square, m * n);

	/* print dead end boards; by row */
	printf("THREAD %ld: Dead end boards:\n", pthread_self());
	for(int k = 0; k < deadends; k ++){
		if(dead_end_cov[k] >= args.x){
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
	}

	/* free all you can free here */

	// free(global_tid_l);

	// for(int i = 0; i < thread_no; i ++){
	// 	for(int j = 0; j < m; j ++){
	// 		free(dead_end_boards[i][j]);	
	// 	}
	// 	free(dead_end_boards[i]);
	// }	
	// free(dead_end_boards);


	/*********/


	return EXIT_SUCCESS;

}

