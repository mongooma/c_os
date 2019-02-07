#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

// FILE *stderr;  //use this when under win

/*
implement a rudimentary cache of words found in a given text ﬁle 
*/

int hash(char * word, int div) {  
	
	int total = 0;
	for (int i = 0; i < strlen(word); i++) { 
		total += *(word + i); /* brackets! */
	}
	return total % div; 

}
int main(int argc, char **argv) { 
	/* 
	The ﬁrst command-line argument speciﬁes the size of the cache;
	open and read the regular ﬁle speciﬁed as the second command-line argument
	*/

	/* ANSI C standard: every declaration at the top of the scope before any executable */
	char *input;
	char **cache; /* list of pointers */
	FILE *input_file;	/*read and parse the text in the file refered by the filename*/
	char *w;
	int tmp;
	int flag;
	int index;
	int i;
	int tmp_l; /* check word length */
	/* --- end of declaration */

	setvbuf( stdout, NULL, _IONBF, 0 );


	/* If improper command - line arguments are given, report an error message to stderr and abort further program execution */
	
	if (argc < 2) {
		fprintf(stderr, "ERROR: Must provide a cache size and a filename. \n");
		/* or perror("ERROR: <error-text-here>") */
		return EXIT_FAILURE; /* or exit(-1) */
	}

	if (access(*(argv + 2), F_OK) == -1) {  
		fprintf(stderr, "ERROR: file not exist. \n");
		return EXIT_FAILURE; /* or exit(-1) */
	}

	
	if (atoi(*(argv+1)) == 0 && **argv != '0') {  

		fprintf(stderr, "ERROR: invalid cache size. \n");
		return EXIT_FAILURE; /* or exit(-1) */
	
	}

	/* end of checking input*/
	

	input = *(argv + 2); 

	cache = calloc(atoi(*(argv+1)), sizeof(char *)); /* size of char *: each cell contains a pointer (not char) */
												   /*use calloc instead of malloc*/

	input_file = fopen(input, "r");

	/*parse all words from the given ﬁle (if any), determine the cache array index for each word 
	(in the order encountered), 
	then store the word in the cache, replacing any existing word if a collision occurs*/

	w = calloc(128, sizeof( char )); /* buffer */
	if (w == NULL) {
		printf("Memory error. Check. \n");
		return EXIT_FAILURE;
	}
#ifdef DEBUG
		printf("%s, ahha\n", w);
#endif // DEBUG

	flag = 0; // if flag doesn't start with 0, then w will be zero (?)
	tmp_l = 0;
	while((tmp = fgetc(input_file)) != EOF){ 
		/* using fgetc() to read char by char*/
		/* char in c naturally is an ascii INT*/
		/* use scanf to read one word at a time (however need to deal with the punctuations) */

		/*include words and numbers*/
		if ((tmp >= 65 && tmp <=90) || (tmp >= 97 && tmp <= 122) || (tmp >= 48 && tmp <= 57)) {
			*(w + flag) = tmp; 
#ifdef DEBUG
			*(w + flag + 1) = '\0';
			printf("%s here2\n", w);
#endif // DEBUG
			flag += 1;
			tmp_l += 1;
#ifdef DEBUG
			printf("%d here4\n", flag);
#endif // DEBUG
			continue;
		}
		else if(*w != 0 && tmp_l >= 3) { /*todo: check if the word has length larger than 3*/
			 /* when is out of a word  & word length larger than 3; not at the beginning blanks &*/

			*(w + flag) = '\0';
#ifdef DEBUG
			printf("%s here\n", w);
#endif // DEBUG
			index = hash(w, atoi(*(argv + 1)));


			if (*(cache + index) == NULL) { 

				*(cache + index) = calloc(strlen(w) + 1, sizeof(char)); /* remember to allocate \0 for the word*/
				/* use strlen instead of size, as size is for the whole list's length*/
				strcpy(*(cache + index), w);
				printf("Word \"%s\" ==> %d (calloc)\n", w, index);

			}
			else { // replacing the word if collision occurs

				*(cache + index) = realloc(*(cache + index), sizeof(char) * (strlen(w) + 1));
				strcpy(*(cache + index), w);
				printf("Word \"%s\" ==> %d (realloc)\n", w, index);
			}

			/* re-initialize everything */
			flag = 0;
			tmp_l = 0;
			free(w); /* might be an easier way for this*/
			w = calloc(128, sizeof(char)); /* buffer */

		}
		else { /* if there are some blanks at the beginning OR the word length less than 3*/
#ifdef DEBUG
			printf("%s here1\n", w);
#endif // DEBUG
			/* re-initialize everything */
			flag = 0;
			tmp_l = 0;
			free(w); /* might be an easier way for this*/
			w = calloc(128, sizeof(char)); /* buffer */
		}
	}

	for (i = 0; i < (atoi(*(argv+1))); i++) { 
		if(*(cache+i) != NULL){
			printf("Cache index %d ==> \"%s\"\n", i, *(cache+i)); /* brackets! */
		}
		free(*(cache + i)); /* have to be freed individually for each block*/
	}

	fclose(input_file);
	free(cache);
	free(w);

}