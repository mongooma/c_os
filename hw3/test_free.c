#include <stdlib.h>
#include <stdio.h>

int main(){

	int ** l = calloc(3, sizeof(int *));

	l[0] = calloc(1, sizeof(int));
	l[0][0] = 0;
	l[1] = calloc(1, sizeof(int));
	l[1][0] = 1;
	l[2] = calloc(1, sizeof(int));
	l[2][0] = 2;

	
	free(l); // seg fault

	for(int i =0; i < 3; i ++){
		printf("%d\n", l[i][0]);
	}




}

