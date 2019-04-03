#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


typedef struct moves_types
{
	int move_no;
	int * directions;
} moves_type;

typedef struct move_next_args
{
	char ** board; 
	int board_size[2]; 
	int * current_pos; 
	int move;
	int direction;
	int x;
	pthread_t parent_tid;
} move_next_arg;

/* global variables */
int max_square;
int deadends;
long int * global_tid_l;
long int global_tid_l_len;
int thread_no;
char *** dead_end_boards;
int * dead_end_cov;

void * move(void * arguments);
void check_move(moves_type * available, const char ** board, const int * board_size, const int * current_pos);
int move_to_direction(char ** board, int * current_pos, const int direction);