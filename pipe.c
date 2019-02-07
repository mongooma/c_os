#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(){
	int p[2];
	int rc = pipe(p); /* p will be filled with descriptors*/
	/*fd
	 *	0 stdin
	 *	1 stdout
	 *	2 stderr
	 *	3 p[0] <----read----   |buffer| temporary (will automaticaly cleared after exit) 
	 *	4 p[1] ---write----->  |      |
	 * */
	
	if(rc == -1){
		perror("pipe() failed!");
		return EXIT_FAILURE;
	
	}
	
	printf("p[0]: %d, p[1]: %d \n", p[0], p[1]);

	pid_t pid = fork(); /* shard fd: 3 for p[0] and 4 for p[1], same read/write end*/
	if (pid == -1){
	
		perror("fork() failed \n");
		return EXIT_FAILURE;
		
	}
	if (pid == 0){ /*child process*/
		
		//sleep(10);
		close(p[0]);
		int bytes_written = write(p[1], "whisncihiasihe", 100);
		printf("wrote %d bytes to the pipe \n", bytes_written); /*sometimes will show after "read"*/
		
		close(p[1]);

        }else{
		close(p[1]);
 		char buffer[100];               
       		int bytes_read = read(p[0], buffer, 5);
		buffer[bytes_read] = '\0';
		printf("read %d bytes: %s \n", bytes_read, buffer);

        	bytes_read = read(p[0], buffer, 5);
        	buffer[bytes_read] = '\0';
        	printf("read %d bytes: %s \n", bytes_read, buffer);
		close(p[0]);
	}

	//close(p[0]);
	//close(p[1]);


	return 0;

}
