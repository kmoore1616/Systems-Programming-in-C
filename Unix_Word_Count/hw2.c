#include <stdlib.h>
#include <stdio.h>

void wc(FILE* file, int* out){
	int line_count = 0;
	int word_count = 0;
	int char_count = 0;
	int buffsize = 1000;
	char mybuff[buffsize];
	int i = 0;
	char last_char_spc = 0; // Doesn't count a new word for subsequent spaces
	
	while (fgets(mybuff, buffsize, file) != NULL) {
		line_count++;
		if(mybuff[0] != '\n'){
			word_count++;
		}
		while(mybuff[i] != '\n'){
			char_count++;
			i++;
			if (mybuff[i] == ' ' && !last_char_spc) {
                last_char_spc = 1;
                word_count++;
            } else if (mybuff[i] != ' ') {
                last_char_spc = 0;
            }
		}
		char_count++;
		i=0;
	}
	out[0] = line_count;
	out[1] = word_count;
	out[2] = char_count;
}

int main(int argc, char** argv) {
    // Your code here
	
	if(argc < 2){
		printf("Error... Usage ./wc file1.txt file2.txt ...");
		return 1;
	}

	FILE* target;
	int** results;	// Holds each item returned by wc
	results = (int**) malloc(sizeof(int*)*argc);	// Dynamically allocate array to size=argc

	int i, j;
	char multi_file = (argc > 2) ? 1 : 0; 
	int tot_lines = 0;
	int tot_words = 0;
	int tot_chars = 0;

	for(i=1; i<argc; i++){	
		target = fopen(argv[i], "r");
		results[i - 1] = (int*)malloc(3 * sizeof(int));
		wc(target, results[i-1]);
		fclose(target);
	}

	for (i = 1; i < argc; i++) {
    	printf("\t%d\t%d\t%d %s\n", results[i - 1][0], results[i - 1][1], results[i - 1][2], argv[i]);
    	for (j = 0; j < 3; j++) {
        	switch (j) {
            	case 0:
                	tot_lines += results[i - 1][j];
                	break;
            	case 1:
                	tot_words += results[i - 1][j];
                	break;
            	case 2:
                	tot_chars += results[i - 1][j];
                	break;
        	}
    	}

    	free(results[i - 1]);
	}
	if(multi_file){
		printf("\t%d\t%d\t%d total\n", tot_lines, tot_words, tot_chars);
	}


	free(results);
    return 0;
}

