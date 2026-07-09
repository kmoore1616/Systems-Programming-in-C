#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#define WORD_LEN 100

char super_secret_password[WORD_LEN];   // Where password is stored
int word_count; 
int found = 0;
char solution[WORD_LEN*2];
char** words;
char* buffer;

// Counts the number of words in a string by a delimiter
int count_words(const char* str, char delimit) {
    int count = 0;  // Count of words
    int in_word = 0;   // Keeps track of if you in a word 
    
    // While < end of file
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == delimit) {    // Wait until delimiter when in a word
            in_word = 0;
        } else if (!in_word){   // Otherwise add to count and reset in_word
            in_word = 1;
            count++;
        }
    }
    return count;
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
        if (i >= arg_count) {
            fprintf(stderr, "Too many tokens for arg_vars!\n");
            return -1;
        }
        arg_vars[i] = token;
        token = strtok(NULL, &delimit);
        i++;
    }
    arg_vars[arg_count] = NULL;
    return 1;

}

// Driver function to process dict into a char** array "words"
int process_dict(int dict_fd, char*** words_out){
    buffer = malloc(1000001);   // Buffer needs to survive return 
    int char_count = read(dict_fd, buffer, 1000001);    // read from dict
    
    if(char_count == -1){
        perror("Error reading dict...");
        return -1;
    }
    
    buffer[char_count] = '\0';  // Null terminate buffer
    word_count = count_words(buffer, '\n'); // Count how many \n's in buffer. (Should have just used strtok)
    char** temp_words = (char**) malloc(sizeof(char*)*word_count+1);   // Allocate array to store all words 
    if(process_args(buffer, temp_words, word_count, '\n') == -1){   // Fill array
        printf("Error processing buffer...\n");
        return -1;
    }
    
    *words_out = temp_words;    // Modify pass by reference
    return 0;
}

// Uses threads to find a password. arg signifies where the thread should start, and how many words it needs to try
void* pass_search(void* arg){
    int* assignment = (int*)arg;    // Cast argument to correct type
    char word_1[WORD_LEN];  // Word one of guess
    char word_2[WORD_LEN];  // Word two of guess
    char guess[WORD_LEN*2]; // Actual guess to check against super_secret_password
                            //
    // 2d loop to try every possible solution. When "found" is set, skips checks to hasten thread termination
    for (int first = assignment[0]; first < assignment[1]; first++){    
        if (found) break;   // Stop checking once password is found
        strcpy(word_1, words[first]);   // Grab first guess
        
        for (int second = 0; second < word_count; second++) {
            if (found) break;   // Stop checking once password is found
            strcpy(word_2, words[second]);  // Grab second guess 
            
            sprintf(guess, "%s%s", word_1, word_2);     // Combine
            if (strcmp(guess, super_secret_password) == 0) {    // Is guess correct?
                strcpy((char*)solution, guess);     // Yes? Set solution and found flag
                found = 1;
                break;
            }   // Otherwise keep checking
        }
    }
    
    free(assignment);   // Cleanup
    return NULL;
}

// Picks a random password
int pick_random(int mod, char** words){
    sprintf(super_secret_password, "%s%s", words[rand()%mod], words[rand()%mod]);
    return 0;
}


        

int main(int argc, char** argv){
    // Is input correct?
    if(argc!=4){
        printf("Error... Usage ./hw12 filename seed num_threads\n");
        return -1;
    }
    char* file_name = argv[1]; 
    int seed = atoi(argv[2]);
    int num_threads = atoi(argv[3]);
  
    // If user enters seed = -1, a random number is generated
    if(seed == -1){
        srand(time(NULL));
    }else{
        srand(seed);  
    }

    int retval = open(file_name, O_RDONLY); // Open dict
    if(retval == -1){
        perror("Error opening dict...");
        return -1;
    }
    
    if(process_dict(retval, &words) == -1){ // Process dict into char** array "words"
        printf("Error processing dict...\n");
        return -1;
    }
    close(retval);  // Close file

    pick_random(word_count, words); // Assign password
    
    pthread_t* thread_ids = (pthread_t*)malloc(sizeof(pthread_t) * num_threads);    // Malloc thread id array
    int words_per_thread = word_count / num_threads;    // Calculate words per thread. Used for word assignment

    for(int i=0; i<num_threads; i++){   // Spin up and set threads upon their work
        int* assignment = malloc(2 * sizeof(int));  // Argument to be passed to thead function
        assignment[0] = i * words_per_thread;   // Starting position in dictionary
        assignment[1] = (i == num_threads - 1) ? word_count : (i + 1) * words_per_thread;   // Number of words to check
        pthread_create(&thread_ids[i], NULL, pass_search, (void*)assignment);   // Start thread and direct to pass_search
    }
   
    for (int i = 0; i < num_threads; i++) { // Wait for threads to finish
        pthread_join(thread_ids[i], NULL);
    }
    

    if(found){
        printf("Password found! - %s\n", solution);     // Print out password found
    }else{
        printf("Something went wrong...\n");    // Otherwise ):
    }
    
    // Cleanup
    free(buffer);
    free(thread_ids);
    free(words);
    return 0;

}

