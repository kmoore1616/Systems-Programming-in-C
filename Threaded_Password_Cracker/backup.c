#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>

#define DICT_SIZE 1000000

char super_secret_password[100]; 
volatile int found = 0;
char** words;
char solution[100];

void* fx(void *arg){
    int* assignment = (int*)arg;
    char word_1[100];
    char word_2[100];
    char guess[200];

    for(int first=assignment[0]; first<assignment[1]; first++){
        if (found) break;
        strcpy(word_1, words[first]); 
        for(int second = assignment[0]; second<assignment[1]; second++){
            if (found) break;
            strcpy(word_2, words[second]); 
            sprintf(guess, "%s%s", word_1, word_2);
            if(strcmp(guess, super_secret_password) == 0){
                strcpy(solution, guess);
                found = 1;
                break;
            }
        }
    }
    free(assignment);
    return NULL;
    
}

int get_rand(int mod){
    return rand() % mod;
}


int main(int argc, char** argv){
    if(argc != 4){
        printf("Error... Usage ./hw12 file.name seed num_threads\n");
        return -1;
    }

    int num_threads = atoi(argv[3]);
    int i=0;
    int word_count = 0;
    
    int dict_fd;
    int wpt;
    int char_count;
    
    int random_indicies[2];
    char dict[DICT_SIZE]; 
    
    if(atoi(argv[2])>0){
        srand(atoi(argv[2]));
    }else{
        srand(time(NULL));
    }

    dict_fd = open(argv[1], O_RDONLY);
    if(dict_fd == -1){
        perror("Error open dict...");
        return -1;
    }
    char_count = read(dict_fd, dict, DICT_SIZE);
    if(word_count == -1){
        perror("Read fail...");
        return -1;
    }
    dict[char_count] = '\0';
    while(dict[i] != '\0'){
        if(dict[i] == '\n'){
            word_count++;
            }
        i++;
    }
    words = (char**)malloc(word_count * sizeof(char*));
   
    i=0;
    char* token = strtok(dict, "\n");
    while (token != NULL && i < word_count) {
        words[i++] = token;
        token = strtok(NULL, "\n");
    }

    for(int i=0; i<2; i++){
        random_indicies[i] = get_rand(word_count-1);
    }
    sprintf(super_secret_password, "%s%s", words[random_indicies[0]], words[random_indicies[1]]);
    printf("%s\n", super_secret_password);

    close(dict_fd);

    // Threadstuff
    wpt = word_count / num_threads;

    pthread_t* thread_ids = (pthread_t*) malloc(sizeof(pthread_t)*num_threads);
   
    // Spin up threads
    for(int i=0; i<num_threads; i++){
        int* assignment = malloc(2 * sizeof(int));
        assignment[0] = i*wpt;
        assignment[1] = (i == num_threads - 1) ? word_count : (i + 1) * wpt;;
        pthread_create(&thread_ids[i], NULL, fx, (void*)assignment);
    }

    for(int i=0; i<num_threads; i++){
        pthread_join(thread_ids[i], NULL);
    }

    if(found){
        printf("Password found! - %s\n", solution);
    }else{
        printf("Nothing found...\n");
    }

    free(words);
    return 0;
}

