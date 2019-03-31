#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "functions.h"

/* You can't execute code outside of functions.*/
int max_square;
int deadends;
pthread_t * global_tid_l;
int global_tid_l_len;
int thread_no;
char *** dead_end_boards;

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

typedef struct moves_type
{
	int move_no;
	int * directions;

};

typedef struct move_next_args
{
	char ** board; 
	const int * board_size; 
	int * current_pos; 
	const int direction;
};


int move_next(void * arguments){

	/*
	param board: a current board configuration at this leaf
	param current_pos: (i, j) -> row, col, start from up-left corner
	param board_size: (m, n) -> row, col

					 +1

         +0

			     *
			
		     	        +2		
           
            +3		

    return: 
       each child thread that's a leaf node returns the number of squares covered, i.e., as far as it can get -- 
	   
	   for any intermediate nodes, those threads return the maximum value received from their child threads.

	*/

	struct move_next_args * args = arguments;

	struct move_next_args ** new_args_l = calloc(4, sizeof(move_next_args *));

	/* 1. make move */

	move_to_direction(arguments.board, arguments.board_size, arguments.current_pos, arguments.direction);
	/* board configuration and current_pos are changed*/

	/* 2. check next available move no */

	moves_type available = check_move(arguments.board, arguments.board_size, arguments.current_pos);
	assert(available.move_no <= 4);

	char *** new_board_l;

	/* if valid next move, create threads */
	if(available.move_no > 1){
		int max_covered = 0;

		for(int i = 0; i < 4; i ++){
			
			if(available.directions[i] == 1){

				new_board_l[i] = calloc(board_size[0], sizeof(char *));
				for(int j = 0; j < board_size[1]; j ++){
					new_board_l[i][j] = calloc(board_size[1], sizeof(char));
				} 

				new_args_l[i] = arguments;
				new_args_l[i].direction = i;
				pthread_t * tid;

				/* add the tid to the global tid list*/				
				pthread_mutex_lock( &mutex_2 );
				if(thread_no >= global_tid_l_len){
					global_tid_l = realloc(2 * global_tid_l_len, sizeof(pthread_t));
				}
				pthread_mutex_unlock( &mutex_2 );
				
				pthread_create(tid, NULL, move_next, (void *)&new_args_l[i]);
				
				pthread_mutex_lock( &mutex_3 );
				global_tid_l[thread_no-1] = tid;
				pthread_mutex_lock( &mutex_3 );
				
				#ifdef NO_PARALLE
				int covered;
				pthread_join(*tid, &covered);
				if(covered > max_covered){
					max_covered = covered;
				}
				#endif

			}

		}

		/********************/
		free(new_board_l); /* will it affect the new boards in the list?*/
		/*free the old board */
		for(int i = 0; i < board_size[1]; i ++){
			free(board[i]);
		} 
		free(board);

		return max_covered; /* if use NO_PARALLEL this will be meaningful? */
		/*********************/

	}else if(available.move_no == 1){
		/* if only one valid move, no child thread is created*/

		for(int i = 0; i < 4; i ++){	
			if(available.directions[i] == 1){
				arguments.direction = i;
				move_next(arguments);
				break;
			}
		}

		return 0

	}else{ /* dead end*/
		/* todo: dead end boards....*/
		pthread_mutex_lock( &mutex_1 );
		/* add to dead_end_squares */
		deadends += 1;
		dead_end_boards = realloc(deadends, sizeof(char **));
		dead_end_boards[deadends - 1] = calloc(board_size[0], sizeof(char *))
		for(int i = 0; i < board_size[0]; i++){
			dead_end_boards[deadends - 1][i] = calloc(board_size[1], sizeof(char));
		}
		for(int i = 0; i < board_size[0]; i ++){
			for(int j = 0; j < board_size[1]; j ++){
				dead_end_boards[deadends - 1][i][j] = board[i][j];
			}
		}

		/* update max_square */

		if(max_square < covered){
			max_square == covered;
		}
		
		pthread_mutex_unlock( &mutex_1 );	

		/*free the old board */
		for(int i = 0; i < board_size[1]; i ++){
			free(board[i]);
		} 
		free(board);

		/* report dead end and return the no. of squares covered*/
		return covered;
		/*********************/
	
	}

}



moves check_move(const char ** board, const int * board_size, const int * current_pos){
	/*
	
	return moves_type:

		- move_no
		- directions, int * [, , , ]

				  +1

     +0

		     *
		
	     	        +2		
       
        +3			
 

	*/

	moves_type available;
	available.directions = calloc(4, sizeof(int));
	available.move_no = 0;

	/* check 0 */
	if((current_pos[0] > 0) & (current_pos[1]) > 1){
		if(board[current_pos[0] - 1][current_pos[1] - 2] != 'S'){
			available.directions[0] = 1;
			available.move_no += 1;
		}
	}

	/* check 1 */
	if((current_pos[0] > 2) & (current_pos[1]) < board_size[1]){
		if(board[current_pos[0] - 2][current_pos[1] + 1] != 'S'){
			available.directions[1] = 1;
			available.move_no += 1;
		}
	}

	/* check 2 */
	if((current_pos[0] < board_size[0]) & (current_pos[1]) < (board_size[1] - 1)){
		if(board[current_pos[0] + 1][current_pos[1] + 2] != 'S'){
			available.directions[2] = 1;
			available.move_no += 1;
		}
	}

	/* check 3 */
	if((current_pos[0] < (board_size[0] - 1)) & (current_pos[1]) > 0){
		if(board[current_pos[0] + 2][current_pos[1] - 1] != 'S'){
			available.directions[3] = 1;
			available.move_no += 1;
		}
	}

	return available;


}




int move_to_direction(char ** board, const int * board_size, int * current_pos, const int direction){
	/*

	 modify the board configuration;

	 modify current_pos (i, j) - >  row, column


				  +1

     +0

		     *
		
	     	        +2		
       
        +3			
 


	*/

	switch(direction){

		case 0:
		// row - 1, col - 2
			current_pos[0] -= 1;
			current_pos[1] -= 2;

		case 1:
		// row - 2, col + 1
			current_pos[0] -= 2;
			current_pos[1] += 1;

		case 2:
		// row + 1, col + 2
			current_pos[0] += 1;
			current_pos[1] += 2;

		case 3:
		// row + 2, col - 1
			current_pos[0] += 2;
			current_pos[1] -= 1;

	}


	board[current_pos[0]][current_pos[1]] = 'S';


}
