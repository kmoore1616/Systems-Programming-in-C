#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "mfile.h"

const int standard_permis = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
int flags;

mFILE *mfopen(const char *fileName, const char *mode){
	flags = 0 | O_CREAT;	// Make sure flags are clear
	if(*mode == 'r'){
		flags |= O_RDONLY;	// If 'r' set to readonly
	}else if(*mode == 'w'){
		flags |= O_WRONLY;	// If 'w' set to writeonly
	}else if(*mode == 'a'){
		flags |= O_WRONLY | O_APPEND;;	// If 'a' set to writeonly and append
	}else{
		printf("Error... Mode must be 'w', 'r', or 'a'");	// Otherwise throw error and exit
		return NULL;
	}

	// Allocate space for new file
	mFILE* new_f = (mFILE*) malloc(sizeof(mFILE));
	//open newfile
	new_f->fd = open(fileName, flags, standard_permis); 
	new_f->flags = flags;
	if(new_f->fd < 0){
		fprintf(stderr, "Error.. %d\n", errno);
		free(new_f);
		return NULL;
	}
	return new_f;
}

// void *ptr -> buffer to read into, size is sizeof(datatype entered), nitems is how many items to read in.)
int mfread(void *ptr, size_t size, size_t nitems, mFILE *fptr){
	// Read bytes to *ptr 
	ssize_t readsize;
	char* buffer = (char*) malloc(size*nitems);
	readsize = read(fptr->fd, buffer, nitems*size);
	memcpy(ptr, buffer, readsize);
	return readsize;
}	


int mfwrite(void *ptr, size_t size, size_t nitems, mFILE *fptr){
	char buffer[size*nitems];
	memcpy(buffer, ptr, nitems*size);
	
	ssize_t wsize = write(fptr->fd, buffer, nitems*size);
	return wsize;

}

int mfclose(mFILE *fptr){
	int balls = close(fptr->fd);
	if(balls == -1){
		fprintf(stderr, "Error... %d\n", errno);
		return NULL;
	}else{
		free(fptr);
		return 0;
	}
}
