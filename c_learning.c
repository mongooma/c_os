#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h> 
#include <string.h>
#include <ctype.h>
#include <unistd.h> //for unix
#include <errno.h>
#include <sys/wait.h>


/* change a variable by passing its pointer to the func */
int change_arr(char ** arr){
	/* passed */

	arr[0] = calloc(3, sizeof( char ));
	arr[0][0] = 'i';
	arr[0][1] = '\0';
	return 0;

}

char ** change_arr_1(char ** argv_user) {
	/* passed */
	/* couldn't replicate the error with hw2 getCmd */

	char * arg_buffer = calloc(10, sizeof(char)); 
	
	arg_buffer[0] = 'I';
	arg_buffer[1] = '\0';
			
	for(int k=0; k < 100; k ++){
		argv_user = realloc(argv_user, (k+1) * sizeof( char * )); 
		argv_user[k] = calloc(strlen(arg_buffer) + 1, sizeof( char ));
		strcpy(argv_user[k], arg_buffer); 
	}

	free(arg_buffer); 

	return argv_user; 
}

/* this function: if we don't explicitly return argv_data, d.argv_user[0] will be modified after the function returns*/
/* try to replicate this bug*/

typedef struct {

	int * argv_no;
	char ** argv_user;

}argv_data;

argv_data getCmd(const char * buffer, argv_data d) {
	/* 	:param const char * buffer 	user input
		:param argv_data d 			a struct to store separate cmds
		:return argv_data d
	*/
	/* read buffer and store each argument in char ** argv*/
	/* argv has initial 1 block and will be extended after this function:
		*/
	/* 
		Output: length of argv k+1
	*/

	/* debugged 30min */

	char *arg_buffer = calloc(1024 + 1, sizeof(char)); /* temp, freed after*/

	int i = 0; 
	int j = 0;
	int k = 0;
	
	while (buffer[i] != '\0') {
		if (isspace(buffer[i])) {
			
			arg_buffer[j] = '\0';
			d.argv_user[k] = calloc(strlen(arg_buffer) + 1, sizeof(char));
			strcpy(d.argv_user[k], arg_buffer); 
			
			k++; // for argv
			d.argv_user = realloc(d.argv_user, (k+1) * sizeof(char *)); /* extend one block for argv, the new block is not initialized*/
			j = 0;

		}
		else {
			
			arg_buffer[j] = buffer[i]; /* bad pointer assignment (?)*/
			j++; // for argv_buffer

		}
		i++; 
	}
	/*dealing with the last argument*/

	arg_buffer[j] = '\0';
	d.argv_user[k] = calloc(strlen(arg_buffer) + 1, sizeof(char)); 
	strcpy(d.argv_user[k], arg_buffer); 


	#ifdef DEBUG
	for(int i = 0; i < k+1; i ++){
		printf("%s len: %d\n", d.argv_user[i], (int) strlen(d.argv_user[i]));
	}
	#endif

	d.argv_no[0] = (int)(k+1);

	free(arg_buffer); //this free will not change argv
	return d;
}

/**********************************************************/


/* decide about the length of an array */

int arr_len(){

	char * arr[2] = {"hi", "how"};
	int l = (int) (sizeof(arr)/sizeof(char *)); /* 1 */
	printf("len: %d \n", l);

	for( int i = 0; i < l; i ++ ){

		printf("%s\n", arr[i]);
	}

}

/* char array initialization */

int char_arr_initialization(){
	/* try some bracket and string initialization */

	/* String initialization*/
	char * arr = "hi";
	printf("arr: strlen %d, size %d \n", (int) strlen(arr), (int) (sizeof(arr)/sizeof(char)));
	char * arr_1 = "hiiiiiiiii";
	printf("arr: strlen %d, size %d \n", (int) strlen(arr_1), (int) (sizeof(arr_1)/sizeof(char)));

	/* arr: strlen 2, size 8
	arr: strlen 10, size 8 */

	char arr1[] = "hi"; 
	char arr1_1[] = "hi\0"; 
	printf("arr1: strlen %d, size %d \n", (int) strlen(arr1), (int) (sizeof(arr1)/sizeof(char)));
	printf("arr1_1: strlen %d, size %d \n", (int) strlen(arr1_1), (int) (sizeof(arr1_1)/sizeof(char)));
	arr1[0] = 'o';
	printf("arr1[0]: %c \n", arr1[0]);
	/* arr1: strlen 2, size 3
	arr1_1: strlen 2, size 4 */
	// char * new_arr1 = calloc(5, sizeof(char));
	// new_arr1 = realloc(arr1, 5); /* (runtime error) invalid pointer error: arr_1 is not dynamically allocated*/

	char arr2[2] = "hi"; //uncertain strlen, no '\0'; size 2
	char arr2_1[3] = "hi\0";
	printf("arr2[0]: %c, arr2[1]: %c \n", arr2[0], arr2[1]);
	printf("arr2_1[0]: %c, arr2_2[1]: %c \n", arr2_1[0], arr2_1[1]);
	printf("arr2: strlen %d, size %d \n", (int) strlen(arr2), (int) (sizeof(arr2)/sizeof(char)));
	printf("arr2_1: strlen %d, size %d \n", (int) strlen(arr2_1), (int) (sizeof(arr2_1)/sizeof(char)));
	
	arr2[0] = 'o';
	printf("arr2[0]: %c \n", arr1[0]);
	
	/*arr2[0]: h, arr2[1]: i
	arr2_1[0]: h, arr2_2[1]: i
	arr2: strlen 4, size 2
	arr2_1: strlen 2, size 3*/


	/* bracket initialization */
	char arr3[] = {'h', 'i', '\0'};
	arr3[0] = 'o';
	printf("arr3[0]: %c \n", arr3[0]);
	
	char arr3_1[3] = {'h', 'i', '\0'};
	arr3_1[0] = 'o';
	printf("arr3_1[0]: %c \n", arr3_1[0]);
	
}

/* dynamic memory allocation */

int mem_alloc(){
	/* test -1 */
	//char ** arr = malloc(20);
	// strcpy(arr[0], "hi\0"); /* seg fault */ /* could access till arr[16819], arr[3] cannot access memory at arr[3]*/
     	



	/* test 1*/
		// char ** arr = malloc(4 * 5);

		printf("arr[0]: pos %p \n \n", (void *) &(arr[0]));

		arr[2] = malloc(5);
		printf("arr[2]: pos %p \n", (void *)  &(arr[2]));
		arr[0] = malloc(3);
		printf("arr[0]: pos %p \n", (void *) &(arr[0]));
		arr[1] = malloc(4);
		printf("arr[1]: pos %p \n", (void *)  &(arr[1]));
		printf("arr[2]: pos %p \n", (void *)  &(arr[2]));

		/* test 1 output*/
		// // arr[0]: pos 0x8403260
		// // arr[2]: pos 0x8403270
		// // arr[0]: pos 0x8403260
		// // arr[1]: pos 0x8403268
		// // arr[2]: pos 0x8403270
		
		// arr_1[3] = malloc(20); // no error
		/* test 1 end */

	/* test 2 */
		char ** arr = malloc(1);
	    

		printf("arr[0]: pos %p \n \n", (void *) &(arr[0]));

		arr[2] = malloc(5);
		printf("arr[2]: pos %p \n", (void *)  &(arr[2]));
		arr[0] = malloc(3);
		printf("arr[0]: pos %p \n", (void *) &(arr[0]));
		arr[1] = malloc(4);
		printf("arr[1]: pos %p \n", (void *)  &(arr[1]));
		printf("arr[2]: pos %p \n", (void *)  &(arr[2]));


		/* test 2 output*/ /* the same? i.e. tried malloc(10), all same*/
		/*
		arr[0]: pos 0x8403260

		arr[2]: pos 0x8403270
		arr[0]: pos 0x8403260
		arr[1]: pos 0x8403268
		arr[2]: pos 0x8403270
		*/








}



int main(){

	mem_alloc();

	return 0;
}