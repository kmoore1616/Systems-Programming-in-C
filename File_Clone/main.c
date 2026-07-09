#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
//#include <string.h>
//#include <stdlib.h>

#include "mfile.h"

int main(){
	mFILE *f = mfopen("test.txt", "w");

	mfwrite((void *) "hhellohellohellohellohellohellohellohellohellohellohellohellohellohellohellohellohellohelloello\n", 6, 1, f); 
	mfclose(f);

	char str[256];
	char str2[256];

	f = mfopen("test.txt", "r");

	mfread(str, 6, 1, f); 

	printf("%s\n", str);
	mfclose(f);

	f = mfopen("test.txt", "a");

	mfwrite((void *) "hi\n", 3, 1, f); 

	mfclose(f);

	f = mfopen("test.txt", "r");

	mfread(str, 12, 1, f); 

	printf("%s\n", str);
	mfclose(f);
}
