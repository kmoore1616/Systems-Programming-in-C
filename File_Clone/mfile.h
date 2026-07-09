#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

//STUDENT CODE - create mFILE struct here!

typedef struct mFile {
	int fd;
	int flags;
	
}mFILE;

mFILE *mfopen(const char *fileName, const char *mode);
int mfread(void *ptr, size_t size, size_t nitems, mFILE *fptr);
int mfwrite(void *ptr, size_t size, size_t nitems, mFILE *fptr);
int mfclose(mFILE *fptr);

