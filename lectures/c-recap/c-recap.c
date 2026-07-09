#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


int main(int argc, char ** argv) {
	int *xs;
	int N = 100;

	xs = (int *) malloc(sizeof(int)*N);
	
	int i;
	for(i=0; i<N; i++){
		xs[i] = rand() % 1000;
	}
	for(i=0; i<N; i++){
		printf("xs[%d] => %d\n", i, xs[i]);
	}


	return 0;
}
