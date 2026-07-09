#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

// Counts number of words based on delimitter
int count_words(char* word, char delimit){
    char next_char; 
    char last_char = delimit;
    int wc = 1; // Assume every string has 1 word (empty strings are handled elsewhere)
    for(int i=0; i<strlen(word); i++){  // Loop through all chars
        next_char = word[i];
        if(next_char == delimit && last_char != delimit){   // Skip consequtive spaces
            wc++; 
        }
        last_char = next_char;
    }
    return wc;
}

// Breaks user input into a char** array for use in the execvp call
// Returns -1 on error and 0 on success
int process_args(char* u_in, char** arg_vars, int arg_count, char delimit){
    char* token;
    int i = 0;
    token = strtok(u_in, &delimit);  // Grab first token
    if(token == 0x0){   // If its 0x0 (nothing was passed) ignore command
        return -1;
    }

    while(token != NULL){   // Loop through string and tokenize
        arg_vars[i] = token;
        token = strtok(NULL, &delimit);
        i++;
    }
    arg_vars[arg_count] = NULL;
    return 1;
}


int main(int argc, char** argv){
    char** argument;
    char** commands[argc-1];    // 3d array that keeps track of each call along the pipe 
    int pipes[argc-2][2];   // An array of pipe fd's
    int cmd_size;   // 
    int ret;    // Used to track various return values
    int n = argc-1; // N commands entered

    if(argc == 1){  // If no args are given, just end the program
        return 0;
    }

    // Organize int char** "commands"
    for(int i=1; i<argc; i++){
        char* temp = strdup(argv[i]); // Copy argv[i] so it isnt overwritten
        cmd_size = count_words(temp, '_');  // Counts number of arguments in a command
        commands[i-1] = (char**) malloc(sizeof(char*)*(cmd_size+1));    // Allocate memory for each command
        process_args(temp, commands[i-1], cmd_size, '_');   // Tokenize each command into its own char** and save to commands[i-1]
    }

    // Open all pipes to be used
    for(int i=0; i<n-1; i++){
        ret = pipe(pipes[i]);
    }

    // Create all children, map stdin/stdout accordingly, close all pipes per child, and execvp each command accordinglyu
    for(int i=0; i<n; i++){
        ret = fork();   // Make a new child
        if(!ret){
            //Child
            close(pipes[i][0]);     // Close Read for pipe out
            
            if(i>0){
                close(pipes[i-1][1]);   // Close write for pipe in 
                dup2(pipes[i-1][0],0);  // If not first, read in from last
            }
            if(i<n-1){
                dup2(pipes[i][1],1);    // If not last write to pipe out
            }

            // Pipes are mapped, we can close them
            for (int j = 0; j < n - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Run command
            execvp(commands[i][0], commands[i]);
            printf("Command not found...\n");   // Catch all for bad input
        }
    }

    for (int i = 0; i < n - 1; i++) {   // Parent closes all pipes
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for all children to complete
    for (int i = 0; i < n; i++) {
        wait(NULL);
    }
    
    // Cleanup
    for (int i = 0; i < n; i++) {
        free(commands[i][0]);  // Free the strdup'd string assigned to the first token
        free(commands[i]);
   }
 return 0;
}
