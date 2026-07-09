#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <utime.h>
#include <sys/time.h>

int main(int argc, char** argv) {
	int fd1 = open("text.txt", O_WRONLY | O_CREAT, S_IRWXU);
	if(fd1 == -1){
		fprintf(stderr, "Failed to open...  %s\n", strerror(errno));
		
	}

	struct utimbuf mytimbuf; 

	mytimbuf.actime = 0;
	mytimbuf.modtime = 0;
	
	close(fd1);

	utime("text.txt", &mytimbuf);

	
	if(fd1 == -1){
		fprintf(stderr, "Failed...  %s\n", strerror(errno));
		
	}

	

	return 0;
}
