#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "functions_new.h"

/* mutex */
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_4 = PTHREAD_MUTEX_INITIALIZER;


/* notes:

1. mutex needed:

	- update the max_length
	- add to the dead_end_boards
	- update no. of deadends 
	- update no. of threads + add tid to global_tid_l + update global_tid_l_len

2. figure out dead end join - how does the child thread communicate to the parent thread?

3. keep a global list for tids? (otherwise how does the parent join its child threads? )

4. suport the no_paralle flag

*/


void * move(void * args){

	/*
	
    return: 
       each child thread that's a leaf node returns the number of squares covered, i.e., as far as it can get -- 
	   
	   for any intermediate nodes, those threads return the maximum value received from their child threads.

	*/

	/* arguments is a pointer; use -> for the members */
	move_next_arg * arguments = args;
	
	#ifdef DEBUG_pass
		printf("move: here0, thread %ld, current_pos: (%d, %d) \n", pthread_self(), 
			arguments->current_pos[0], arguments->current_pos[1]);

		for(int k = 0; k < arguments->board_size[0]; k ++){
			for(int i = 0; i < arguments->board_size[1]; i ++){
				if (arguments->board[k][i] == 'S'){
					printf("(%d, %d)", k, i);
				}
			}
		}
		printf("\n");
	#endif	

	move_next_arg * new_args_l = calloc(8, sizeof(move_next_arg));

	#ifdef DEBUG_pass
		printf("move: here1 \n");
	#endif

    /* check available move no */     
    moves_type available;
	available.directions = calloc(8, sizeof(int));
	available.move_no = 0;
	#ifdef DEBUG_pass
		printf("move: here11 \n");
	#endif
	check_move(&available, (const char **) (arguments->board), (const int *) (arguments->board_size), (const int *) arguments->current_pos);     
	#ifdef DEBUG_pass
		printf("move: here12 \n");
	#endif
	assert(available.move_no <= 8);

	/* prepare args for the child thread move()*/
	for(int i = 0; i < 8; i ++){

		new_args_l[i].board = calloc(arguments->board_size[0], sizeof(char *));
		for(int j = 0; j < arguments->board_size[0]; j ++){
			new_args_l[i].board[j] = calloc(arguments->board_size[1], sizeof(char));
			for(int k = 0; k < arguments->board_size[1]; k ++){
				new_args_l[i].board[j][k] = arguments->board[j][k]; // copy old board value to new board for child threads 
			}
		} 

		/* copy the other values of argument one by one to the new_args_l[i] */
		new_args_l[i].board_size[0] = arguments->board_size[0];
		new_args_l[i].board_size[1] = arguments->board_size[1];
		new_args_l[i].current_pos = calloc(2, sizeof(int));
		new_args_l[i].current_pos[0] = arguments->current_pos[0];
		new_args_l[i].current_pos[1] = arguments->current_pos[1];
		new_args_l[i].move = arguments->move;
		new_args_l[i].x = arguments->x;
		new_args_l[i].parent_tid = pthread_self();


		new_args_l[i].direction = i;
	}



	#ifdef DEBUG_pass
		printf("move: here2 \n");
	#endif


	/* if valid next move, create threads */
	if(available.move_no > 1){
		printf("THREAD %ld: %d moves possible after move #%d; creating threads...\n", 
									pthread_self(), available.move_no, arguments->move);
		int * max_covered;
		max_covered = calloc(1, sizeof(int));

		for(int i = 0; i < 8; i ++){
		
			if(available.directions[i] == 1){

				/* make move */

				move_to_direction(new_args_l[i].board, 
							new_args_l[i].current_pos, (const int) (new_args_l[i].direction));
				new_args_l[i].move += 1;

				#ifdef DEBUG_pass
					printf("move: here10 \n");
				#endif
				/* board configuration and current_pos are changed*/

				pthread_t tid = -1;			
				
				pthread_create(&tid, NULL, move, (void *)&new_args_l[i]); // 
				#ifdef DEBUG_pass
					printf("move: here11, child thread %ld \n", tid);
				#endif
				
				// pthread_mutex_lock( &mutex_3 );
				// /* add the tid to the global tid list*/	
				// thread_no += 1;
				// // if(thread_no >= global_tid_l_len){
				// // 	global_tid_l_len = (int) (2 * global_tid_l_len);
				// // 	#ifdef DEBUG
				// // 	printf("move: global tid len: %ld \n", global_tid_l_len);
				// // 	#endif
				// // 	global_tid_l = realloc(global_tid_l, global_tid_l_len);

				// // }
				// global_tid_l[thread_no-1] = tid; // some memory issue here 
				// pthread_mutex_unlock( &mutex_3 );
				
				#ifdef NO_PARALLEL
				unsigned int * covered; //
				pthread_join(tid, (void **)&covered); //
				pthread_mutex_lock( &mutex_4 );
				// return the max covered of the child threads					
				if(*covered > *max_covered){
					*max_covered = *covered;
				}
				printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", pthread_self(), tid, *covered);
				pthread_mutex_unlock( &mutex_4 );

				#endif
				

			}else{
				/* free this arg */
				// for(int j=0; j < new_args_l[i].board_size[0]; j ++){
				// 	free(new_args_l[i].board[j]);
				// }
				// free(new_args_l[i].board);
				// free(new_args_l[i].current_pos);

			}

		}

		/********************/
		//free(new_board_l); /* will it affect the new boards in the list?: yes*/

		
		/*free the old board */
		//for(int i = 0; i < arguments->board_size[1]; i ++){
		//	free(arguments->board[i]);
		//} 
		//free(arguments->board);

		//pthread_exit(&max_covered); /* only use NO_PARALLEL will this be meaningful (?) */
		
		// free(available.directions);

		// /* free old board*/
		// for(int i=0; i < arguments->board_size[0]; i ++){
		// 	free(arguments->board[i]);
		// }
		// //free(arguments->board); // debug
		// free(arguments->current_pos);
		// //free(arguments); // debug

		return max_covered;
		/*********************/

	}else if(available.move_no == 1){
		/* if only one valid move, no child thread is created*/


		#ifdef DEBUG_pass
			printf("move: here1, move_no==1 \n");
		#endif
		for(int i = 0; i < 8; i ++){	
			if(available.directions[i] == 1){
				arguments->direction = i;
				break;
			}
		}

		move_to_direction(arguments->board, 
					arguments->current_pos, (const int) (arguments->direction));
		arguments->move += 1;

		int * max_covered = calloc(1, sizeof(int));

		max_covered = move(arguments); // have a lock join within

		#ifdef DEBUG_pass
			printf("move: here2, move_no==1 \n");
		#endif


		// free(available.directions);

		// /* free all the new args */
		// for(int i = 0; i < 8; i ++){
		// 	for(int j = 0; j < new_args_l[i].board_size[0]; j ++){
		// 		free(new_args_l[i].board[j]);
		// 	}
		// 	free(new_args_l[i].board);
		// 	free(new_args_l[i].current_pos);
		// }
		// free(new_args_l);

		return max_covered;

	}else{ /* dead end*/
		/* update max_square */
		if(max_square < arguments->move){

			max_square = arguments->move;

		}

		if(arguments->move == (int) (arguments->board_size[0] * arguments->board_size[1])){
			
			/* full coverage is not dead end */

			printf("THREAD %ld: Sonny found a full knight's tour!\n", pthread_self()); // some work-around
		
			// free(available.directions);

			// /* free all the new args */
			// for(int i = 0; i < 8; i ++){
			// 	for(int j = 0; j < new_args_l[i].board_size[0]; j ++){
			// 		free(new_args_l[i].board[j]);
			// 	}
			// 	free(new_args_l[i].board);
			// 	free(new_args_l[i].current_pos);
			// }

			// free(new_args_l);

			/* free old board*/
			// for(int i=0; i < arguments->board_size[0]; i ++){
			// 	free(arguments->board[i]);
			// }
			// //free(arguments->board);
			// free(arguments->current_pos);
			// //free(arguments);

			pthread_exit( &arguments->move ); // this will do

		}

		printf("THREAD %ld: Dead end after move #%d\n", pthread_self(), arguments->move);
		/* todo: dead end boards....*/
		pthread_mutex_lock( &mutex_1 );
		/* add to dead_end_squares */
		deadends += 1;
		// dead_end_boards = realloc(dead_end_boards, deadends); // debug: realloc invalid next size
		// dead_end_boards[deadends - 1] = calloc(arguments->board_size[0], sizeof(char *));
		// for(int i = 0; i < arguments->board_size[0]; i++){
		// 	dead_end_boards[deadends - 1][i] = calloc(arguments->board_size[1], sizeof(char));
		// }
		for(int i = 0; i < arguments->board_size[0]; i ++){
			for(int j = 0; j < arguments->board_size[1]; j ++){
				dead_end_boards[deadends - 1][i][j] = arguments->board[i][j];
			}
			dead_end_cov[deadends - 1] = arguments->move;	
		}
		#ifdef dead_end_check
		fprintf(stderr, "(%d, %d, %d)\n", dead_end_boards[0][0][0], dead_end_boards[0][0][1], dead_end_boards[0][1][0]);
		#endif
		
		pthread_mutex_unlock( &mutex_1 );	

		/*free the old board */
		//for(int i = 0; i < arguments->board_size[1]; i ++){
		//	free(arguments->board[i]);
		//} 
		//free(arguments->board);

		/* report dead end and return the no. of squares covered*/

		#ifdef DEBUG_pass
			printf("move: here3, dead end to join\n");
		#endif


		
		// unsigned int * tmp = calloc(1, sizeof(unsigned int));
		// * tmp = arguments->move;
		// #ifdef DEBUG_pass
		// 	printf("move: here4, dead end to join, covered: %d\n", * tmp);
		// #endif
		//return tmp;

		// free(available.directions);

		// /* free all the new args */
		// for(int i = 0; i < 8; i ++){
		// 	for(int j = 0; j < new_args_l[i].board_size[0]; j ++){
		// 		free(new_args_l[i].board[j]);
		// 	}
		// 	free(new_args_l[i].board);
		// 	free(new_args_l[i].current_pos);
		// }

		// free(new_args_l);

		// /* free old board*/
		// for(int i=0; i < arguments->board_size[0]; i ++){
		// 	free(arguments->board[i]);
		// }
		// //free(arguments->board);
		// free(arguments->current_pos);
		// //free(arguments);

		

		pthread_exit( &arguments->move ); // this will do
		/*********************/
	
	}

}



void check_move(moves_type * available, const char ** board, const int * board_size, const int * current_pos){
	/*
	
	return moves_type:

		- move_no
		- directions, int * [, , , ]

		  2		3

     1               4

		     *
	 8	             5

	     	   
	     7	    6     	
       

	*/
	#ifdef DEBUG_pass
	printf("check_move: before: move_no = %d\n", available->move_no);
	#endif


	/* check 1 */
	if((current_pos[0] > 0) & (current_pos[1] > 1)){
		if(board[current_pos[0] - 1][current_pos[1] - 2] != 'S'){
			available->directions[0] = 1;
			available->move_no += 1;
			#ifdef DEBUG_pass
	printf("check_move: 1 found \n");
	#endif
		}
	}



	/* check 2 */
	if((current_pos[0] > 1) & (current_pos[1] > 0)){
		if(board[current_pos[0] - 2][current_pos[1] - 1] != 'S'){
			available->directions[1] = 1;
			available->move_no += 1;
			#ifdef DEBUG_pass
	printf("check_move: 2 found \n");
	#endif
		}
	}


	/* check 3 */
	if((current_pos[0] > 1) & (current_pos[1] < (board_size[1]-1))){
		if(board[current_pos[0] - 2][current_pos[1] + 1] != 'S'){
			available->directions[2] = 1;
			available->move_no += 1;
			#ifdef DEBUG_pass
	printf("check_move: 3 found \n");
	#endif
		}
	}

	/* check 4 */
	if((current_pos[0] > 0) & (current_pos[1] < (board_size[1] - 2))){
		if(board[current_pos[0] - 1][current_pos[1] + 2] != 'S'){
			available->directions[3] = 1;
			available->move_no += 1;
			#ifdef DEBUG_pass
	printf("check_move: 4 found \n");
	#endif
		}
	}

	/* check 5 */
	if((current_pos[0] < (board_size[0] - 1)) & (current_pos[1] < (board_size[1] - 2))){
		if(board[current_pos[0] + 1][current_pos[1] + 2] != 'S'){
			available->directions[4] = 1;
			available->move_no += 1;
			#ifdef DEBUG_pass
	printf("check_move: 5 found \n");
	#endif
		}
	}

	/* check 6 */
	if((current_pos[0] < (board_size[0] - 2)) & (current_pos[1] < (board_size[1] - 1))){
		if(board[current_pos[0] + 2][current_pos[1] + 1] != 'S'){
			available->directions[5] = 1;
			available->move_no += 1;
			#ifdef DEBUG_pass
	printf("check_move: 6 found \n");
	#endif
		}
	}

	/* check 7 */
	if((current_pos[0] < (board_size[0] - 2)) & (current_pos[1] > 0)){
		if(board[current_pos[0] + 2][current_pos[1] - 1] != 'S'){
			available->directions[6] = 1;
			available->move_no += 1;
			#ifdef DEBUG_pass
	printf("check_move: 7 found \n");
	#endif
		}
	}

	/* check 8 */
	if((current_pos[0] < (board_size[0]-1)) & (current_pos[1] > 1)){
		if(board[current_pos[0] + 1][current_pos[1] - 2] != 'S'){
			available->directions[7] = 1;
			available->move_no += 1;
	#ifdef DEBUG_pass
	printf("check_move: 0 found \n");
	#endif
		}
	}

	#ifdef DEBUG_pass
	printf("check_move: after: move_no = %d\n", available->move_no);
	#endif



}




int move_to_direction(char ** board, int * current_pos, const int direction){
	/*

	 modify the board configuration;

	 modify current_pos (i, j) - >  row, column


		  2		3

     1               4

		     *
	 8	             5

	     	   
	     7	    6    


	*/

	#ifdef DEBUG_pass
		printf("move_to_direction: here \n");
	#endif

	switch(direction){


		case 0:
		// row - 1 , col - 2
			current_pos[0] -= 1;
			current_pos[1] -= 2;
			break;

		case 1:
		// row - 2 , col - 1
			current_pos[0] -= 2;
			current_pos[1] -= 1;
			break;

		case 2:
		// row - 2, col + 1
			current_pos[0] -= 2;
			current_pos[1] += 1;
			break;

		case 3:
		// row - 1, col + 2
			current_pos[0] -= 1;
			current_pos[1] += 2;
			break;

		case 4:
		// row + 1, col + 2
			current_pos[0] += 1;
			current_pos[1] += 2;
			break;

		case 5:
		// row + 2, col + 1
			current_pos[0] += 2;
			current_pos[1] += 1;
			break;

		case 6:
		// row + 2, col - 1
			current_pos[0] += 2;
			current_pos[1] -= 1;
			break;

		case 7:
		// row + 1, col - 2
			current_pos[0] += 1;
			current_pos[1] -= 2;
			break;

	}

	#ifdef DEBUG_pass
		printf("move_to_direction: here1 \n");
	#endif


	board[current_pos[0]][current_pos[1]] = 'S';

	#ifdef DEBUG_pass
		printf("move_to_direction: here2 \n");
	#endif


	return 0;


}
