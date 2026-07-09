#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#define DICT_SIZE 1000000
#define WORD_SIZE 100

char super_secret_password[WORD_SIZE * 2]; 
volatile int found = 0;
char** words;
volatile char solution[WORD_SIZE * 2];
int word_count = 0;

void* fx(void *arg) {
    int* assignment = (int*)arg;
    char word_1[WORD_SIZE];
    char word_2[WORD_SIZE];
    char guess[WORD_SIZE * 2];
    
    for (int first = assignment[0]; first < assignment[1]; first++) {
        if (found) break;
        strcpy(word_1, words[first]); 
        
        for (int second = 0; second < word_count; second++) {
            if (found) break;
            strcpy(word_2, words[second]); 
            
            sprintf(guess, "%s%s", word_1, word_2);
            if (strcmp(guess, super_secret_password) == 0) {
                strcpy((char*)solution, guess);
                found = 1;
                break;
            }
        }
    }
    
    free(assignment);
    return NULL;
}

int get_rand(int mod) {
    return rand() % mod;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("Error... Usage ./hw12 file.name seed num_threads\n");
        return -1;
    }
    
    int num_threads = atoi(argv[3]);
    int dict_fd;
    char* dict;
    long file_size;
    
    // Set random seed
    if (atoi(argv[2]) > 0) {
        srand(atoi(argv[2]));
    } else {
        srand(time(NULL));
    }
    
    // Open dictionary file
    dict_fd = open(argv[1], O_RDONLY);
    if (dict_fd == -1) {
        perror("Error opening dict");
        return -1;
    }
    
    // Get file size
    file_size = lseek(dict_fd, 0, SEEK_END);
    lseek(dict_fd, 0, SEEK_SET);
    
    // Allocate memory and read file
    dict = (char*)malloc(file_size + 1);
    if (!dict) {
        perror("Memory allocation failed");
        close(dict_fd);
        return -1;
    }
    
    if (read(dict_fd, dict, file_size) != file_size) {
        perror("Read failed");
        free(dict);
        close(dict_fd);
        return -1;
    }
    dict[file_size] = '\0';
    close(dict_fd);
    
    // Count words in dictionary
    for (int i = 0; i < file_size; i++) {
        if (dict[i] == '\n') {
            word_count++;
        }
    }
    if (dict[file_size - 1] != '\n') {
        word_count++; // Count last word if no trailing newline
    }
    
    // Allocate words array
    words = (char**)malloc(word_count * sizeof(char*));
    if (!words) {
        perror("Memory allocation failed");
        free(dict);
        return -1;
    }
    
    // Copy dictionary (so we can safely use strtok)
    char* dict_copy = strdup(dict);
    if (!dict_copy) {
        perror("Memory allocation failed");
        free(dict);
        free(words);
        return -1;
    }
    
    // Parse words
    int i = 0;
    char* token = strtok(dict_copy, "\n");
    while (token != NULL && i < word_count) {
        words[i] = strdup(token);
        i++;
        token = strtok(NULL, "\n");
    }
    
    word_count = i; // Adjust word count to actual number of words found
    
    // Create secret password
    int random_indicies[2];
    random_indicies[0] = get_rand(word_count);
    random_indicies[1] = get_rand(word_count);
    
    sprintf(super_secret_password, "%s%s", words[random_indicies[0]], words[random_indicies[1]]);
    printf("Secret password: %s\n", super_secret_password);
    
    // Create threads
    pthread_t* thread_ids = (pthread_t*)malloc(sizeof(pthread_t) * num_threads);
    if (!thread_ids) {
        perror("Memory allocation failed");
        free(dict);
        free(dict_copy);
        for (int j = 0; j < i; j++) {
            free(words[j]);
        }
        free(words);
        return -1;
    }
    
    int words_per_thread = word_count / num_threads;
    
    // Start threads
    for (int i = 0; i < num_threads; i++) {
        int* assignment = malloc(2 * sizeof(int));
        if (!assignment) {
            perror("Memory allocation failed");
            continue;
        }
        
        assignment[0] = i * words_per_thread;
        assignment[1] = (i == num_threads - 1) ? word_count : (i + 1) * words_per_thread;
        
        if (pthread_create(&thread_ids[i], NULL, fx, (void*)assignment) != 0) {
            perror("Thread creation failed");
            free(assignment);
        }
    }
    
    // Wait for threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }
    
    // Report result
    if (found) {
        printf("Password found! - %s\n", solution);
    } else {
        printf("Nothing found...\n");
    }
    
    // Cleanup
    free(thread_ids);
    free(dict);
    free(dict_copy);
    for (int j = 0; j < word_count; j++) {
        free(words[j]);
    }
    free(words);
    
    return 0;
}
