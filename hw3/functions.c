#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "functions.h"

/* mutex */
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;

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

	move_next_arg * arguments = args;
	/* arguments is a pointer; use -> for the members */

	#ifdef DEBUG
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
		printf("move_next: here1 \n");
	#endif

    /* check available move no */     
    moves_type available;
	available.directions = calloc(8, sizeof(int));
	available.move_no = 0;
	#ifdef DEBUG_pass
		printf("move_next: here11 \n");
	#endif
	check_move(&available, (const char **) (arguments->board), (const int *) (arguments->board_size), (const int *) arguments->current_pos);     
	#ifdef DEBUG_pass
		printf("move_next: here12 \n");
	#endif
	assert(available.move_no <= 8);

	/* prepare args for the child thread move()*/
	for(int i = 0; i < 8; i ++){

		new_args_l[i].board = calloc(arguments->board_size[0], sizeof(char *));
		for(int j = 0; j < arguments->board_size[1]; j ++){
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

		new_args_l[i].direction = i;
	}

	#ifdef DEBUG_pass
		printf("move: here2 \n");
	#endif


	/* if valid next move, create threads */
	if(available.move_no > 1){
		printf("THREAD %ld: %d moves possible after move #%d; creating threads...\n", 
									pthread_self(), available.move_no, arguments->move);
		int max_covered = 0;

		for(int i = 0; i < 8; i ++){
		
			if(available.directions[i] == 1){

				/* make move */

				move_to_direction(new_args_l[i].board, 
							new_args_l[i].current_pos, (const int) (new_args_l[i].direction));
				new_args_l[i].move += 1;

				#ifdef DEBUG
					printf("move: here10 \n");
				#endif
				/* board configuration and current_pos are changed*/

				pthread_t tid = -1;

				/* add the tid to the global tid list*/				
				pthread_mutex_lock( &mutex_2 );
				if(thread_no >= global_tid_l_len){
					global_tid_l_len = 2 * global_tid_l_len;
					global_tid_l = realloc(global_tid_l, global_tid_l_len);

				}
				pthread_mutex_unlock( &mutex_2 );
				
				pthread_create(&tid, NULL, move, (void *)&new_args_l[i]); // 
				#ifdef DEBUG
					printf("move: here11, child thread %ld \n", tid);
				#endif
				
				pthread_mutex_lock( &mutex_3 );
				thread_no += 1;
				global_tid_l[thread_no-1] = tid;
				pthread_mutex_lock( &mutex_3 );
				
				#ifdef NO_PARALLEL
				int covered; 
				pthread_join(tid, &covered); //
				if(covered > max_covered){
					max_covered = covered;
				}
				if(covered > arguments->x){
					printf("THREAD %ld: Thread [%d] joined (returned %d)", pthread_self(), *tid, covered);
				}
				#endif

			}

		}

		/********************/
		//free(new_board_l); /* will it affect the new boards in the list?*/
		
		/*free the old board */
		//for(int i = 0; i < arguments->board_size[1]; i ++){
		//	free(arguments->board[i]);
		//} 
		//free(arguments->board);

		pthread_exit(&max_covered); /* only use NO_PARALLEL will this be meaningful (?) */
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
		move(arguments);
		#ifdef DEBUG
			printf("move: here2, move_no==1 \n");
		#endif


		int mid_state = -1;
		pthread_exit(&mid_state);

	}else{ /* dead end*/
		printf("THREAD %ld: Dead end after move #%d\n", pthread_self(), arguments->move);
		/* todo: dead end boards....*/
		pthread_mutex_lock( &mutex_1 );
		/* add to dead_end_squares */
		deadends += 1;
		dead_end_boards = realloc(dead_end_boards, deadends);
		dead_end_boards[deadends - 1] = calloc(arguments->board_size[0], sizeof(char *));
		for(int i = 0; i < arguments->board_size[0]; i++){
			dead_end_boards[deadends - 1][i] = calloc(arguments->board_size[1], sizeof(char));
		}
		for(int i = 0; i < arguments->board_size[0]; i ++){
			for(int j = 0; j < arguments->board_size[1]; j ++){
				dead_end_boards[deadends - 1][i][j] = arguments->board[i][j];
			}
		}

		/* update max_square */

		if(max_square < arguments->move){
			max_square = arguments->move;
		}
		
		pthread_mutex_unlock( &mutex_1 );	

		/*free the old board */
		//for(int i = 0; i < arguments->board_size[1]; i ++){
		//	free(arguments->board[i]);
		//} 
		//free(arguments->board);

		/* report dead end and return the no. of squares covered*/
		pthread_exit(&arguments->move);
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
	 0	             5

	     	   
	     7	    6     	
       

	*/
	#ifdef DEBUG
	printf("check_move: before: move_no = %d\n", available->move_no);
	#endif

	/* check 0 */
	if((current_pos[0] < (board_size[0]-1)) & (current_pos[1] > 1)){
		if(board[current_pos[0] + 1][current_pos[1] - 2] != 'S'){
			available->directions[0] = 1;
			available->move_no += 1;
	#ifdef DEBUG
	printf("check_move: 0 found \n");
	#endif
		}
	}

	/* check 1 */
	if((current_pos[0] > 0) & (current_pos[1] > 1)){
		if(board[current_pos[0] - 1][current_pos[1] - 2] != 'S'){
			available->directions[1] = 1;
			available->move_no += 1;
			#ifdef DEBUG
	printf("check_move: 1 found \n");
	#endif
		}
	}



	/* check 2 */
	if((current_pos[0] > 2) & (current_pos[1] > 0)){
		if(board[current_pos[0] - 2][current_pos[1] - 1] != 'S'){
			available->directions[2] = 1;
			available->move_no += 1;
			#ifdef DEBUG
	printf("check_move: 2 found \n");
	#endif
		}
	}


	/* check 3 */
	if((current_pos[0] > 1) & (current_pos[1] < (board_size[1]-1))){
		if(board[current_pos[0] - 2][current_pos[1] + 1] != 'S'){
			available->directions[3] = 1;
			available->move_no += 1;
			#ifdef DEBUG
	printf("check_move: 3 found \n");
	#endif
		}
	}

	/* check 4 */
	if((current_pos[0] > 0) & (current_pos[1] < (board_size[1] - 2))){
		if(board[current_pos[0] - 1][current_pos[1] + 2] != 'S'){
			available->directions[4] = 1;
			available->move_no += 1;
			#ifdef DEBUG
	printf("check_move: 4 found \n");
	#endif
		}
	}

	/* check 5 */
	if((current_pos[0] < (board_size[0] - 1)) & (current_pos[1] < (board_size[1] - 2))){
		if(board[current_pos[0] + 1][current_pos[1] + 2] != 'S'){
			available->directions[5] = 1;
			available->move_no += 1;
			#ifdef DEBUG
	printf("check_move: 5 found \n");
	#endif
		}
	}

	/* check 6 */
	if((current_pos[0] < (board_size[0] - 2)) & (current_pos[1] < (board_size[1] - 1))){
		if(board[current_pos[0] + 2][current_pos[1] + 1] != 'S'){
			available->directions[6] = 1;
			available->move_no += 1;
			#ifdef DEBUG
	printf("check_move: 6 found \n");
	#endif
		}
	}

	/* check 7 */
	if((current_pos[0] < (board_size[0] - 2)) & (current_pos[1] > 0)){
		if(board[current_pos[0] + 2][current_pos[1] - 1] != 'S'){
			available->directions[7] = 1;
			available->move_no += 1;
			#ifdef DEBUG
	printf("check_move: 7 found \n");
	#endif
		}
	}

	#ifdef DEBUG
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
	 0	             5

	     	   
	     7	    6    


	*/

	#ifdef DEBUG_pass
		printf("move_to_direction: here \n");
	#endif

	switch(direction){

		case 0:
		// row + 1, col - 2
			current_pos[0] += 1;
			current_pos[1] -= 2;
			break;

		case 1:
		// row - 1 , col - 2
			current_pos[0] -= 1;
			current_pos[1] -= 2;
			break;

		case 2:
		// row - 2 , col - 1
			current_pos[0] -= 2;
			current_pos[1] -= 1;
			break;

		case 3:
		// row - 2, col + 1
			current_pos[0] -= 2;
			current_pos[1] += 1;
			break;

		case 4:
		// row - 1, col + 2
			current_pos[0] -= 1;
			current_pos[1] += 2;
			break;

		case 5:
		// row + 1, col + 2
			current_pos[0] += 1;
			current_pos[1] += 2;
			break;

		case 6:
		// row + 2, col + 1
			current_pos[0] += 2;
			current_pos[1] += 1;
			break;

		case 7:
		// row + 2, col - 1
			current_pos[0] += 2;
			current_pos[1] -= 1;
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
