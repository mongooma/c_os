#include <stdlib.h>
#include <stdio.h>


int main(){
	


	int fd = open("test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0660);

	printf("%d created. \n"); /*fd = 3 if created */

	int rc = dup2(fd, 1);

	printf("1,2, 3, 4, 5 \n"); /* this line will be printed to fd*/

	close(fd);
	close(1);


}