#include <stdlib.h>
#include <stdio.h>



typedef struct params{

	int a;
	int b;
	int * c; 
}param;





int main(){

	param p;

	p.a = 1;
	p.b = 2;
	p.c = calloc(2, sizeof(int));
	p.c[0] = 3;
	p.c[1] = 4;


	param p_1; 
	p_1.a = p.a;
	p_1.b = p.b;
	p_1.c = calloc(2, sizeof(int));
	p_1.c[0] = p.c[0];
	p_1.c[1] = p.c[1];

	printf("p.a: %d\n", p.a);
	printf("p.b: %d\n", p.b);
	printf("p.c[0]: %d\n", p.c[0]);
	printf("p.c[1]: %d\n", p.c[1]);

	return 0;

	/*

	param p_1;
	p_1 = p;   // assigning pointer?

	p.a: 1
	p.b: 2
	p.c[0]: 33
	p.c[1]: 44 

	*/


}