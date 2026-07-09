#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

// Counts words in a string. Used for accurate malloc
int count_words(char* word){
    char next_char; 
    char last_char = ' ';
    int wc = 1; // Assume every string has 1 word (empty strings are handled elsewhere)
    for(int i=0; i<strlen(word); i++){  // Loop through all chars
        next_char = word[i];
        if(next_char == ' ' && last_char != ' '){   // Skip consequtive spaces
            wc++; 
        }
        last_char = next_char;
    }
    return wc;
}

// Function that gathers user input
int get_user_in(char* u_in){
    if(write(1, "MY_UNIX> ", 9) == -1){     // Display prompt
        return -1;
    }
    int read_cnt = read(0, u_in, 1000-1);   // Read at max 999 characters (size of maxbuf)
    if(read_cnt == -1){
        return -1;
    }   
    u_in[read_cnt-1] = '\0';    // Add null terminator
    return 0;
}

// Tokenizes string and checks for 'exit' and invalid strings
// Returns 0 for exit, -1 on error
int process_args(char* u_in, char** arg_vars, int arg_count){
    char* token;
    int i = 0;
    token = strtok(u_in, " ");  // Grab first token
    if(token == 0x0){   // If its 0x0 (nothing was passed) ignore command
        return -1;
    }

    if(strcmp(token, "exit") == 0){     // If exit, return 0 (exit)
        return 0;
    }
    while(token != NULL){   // Loop through string and tokenize
        arg_vars[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    arg_vars[arg_count] = NULL;
    return 1;
}


int main(int argc, char** argv){
    int run = 1;
    char u_in[1000];
    char** arg_vars;
    int arg_count;
    int result;

    while(run){     // Main loop
        memset(u_in, 0, sizeof(u_in));  // Clear readbuffer every iteration
        if(get_user_in(u_in) == -1){    // Grab user input
            perror("Error getting user input...");  // Throw error if failed
            exit(-1);
        }
        if(strlen(u_in) == 0){  // If nothing passed ignore command
            continue;
        }
        arg_count = count_words(u_in);  // Grab word count
        arg_vars = (char**) malloc(sizeof(char*) * (arg_count + 1));    // and use in malloc
        result = process_args(u_in, arg_vars, arg_count);   // tokenize args and save result
        if(result == -1){   // If error ignore command
            free(arg_vars);
            continue;
        }else if(result == 0){  // If exit, exit
            free(arg_vars);
            exit(-1);
        } 
        
        int pid = fork();   // Create child
        if(pid==0){     // If child
            execvp(arg_vars[0], arg_vars);  // Run commannd
            printf("MY_UNIX: command not found %s\n", arg_vars[0]); // If execution gets here execvp failed (file not found)
            exit(-1);   // Kill child (parent needs to end wait even if failed)
        }else{
            wait(NULL); // Wait for child. All error checking is already in place
        }

        free(arg_vars); // Free buffer memory (will be allocated next iteration)
    }
    return 0;
}

