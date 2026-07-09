#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Global Variables
double pi=0;    
int circle_points = 0;
int square_points = 0;
int running = 0;

// Monty PIthon estimator
void* monty(void* arg){
    unsigned long int iterations = (unsigned long int)arg;  // Let thread know how many iterations to execute
    double rand_x, rand_y, origin_dist; // Setup monty carlo vars
    
    pthread_mutex_lock(&lock); 
    running = 1; // Basically tell print thread the simulation is beginning
    pthread_mutex_unlock(&lock);
    
    for(int i=0; i<(iterations); i++){  // Performs the monte carlo algoritm
        rand_x = (double) (rand() % (iterations + 1)) / iterations;
        rand_y = (double) (rand() % (iterations + 1)) / iterations;
        origin_dist = rand_x * rand_x + rand_y * rand_y;
        if(origin_dist <=1){
            pthread_mutex_lock(&lock);
            circle_points++;
            pthread_mutex_unlock(&lock);
        }
        pthread_mutex_lock(&lock);
        square_points++;
        pi=(double)(4*circle_points) / square_points;
        pthread_mutex_unlock(&lock);
    }
    
    return NULL;
}

void* print_pi(void* arg){
    int delay = (int)arg;   // Tell thread how long delay should be
    while(!running);    // Wait for a pi thread to begin
    while(running){ // Delay and print value of pi 
        pthread_mutex_lock(&lock);
        double current_pi = pi; 
        pthread_mutex_unlock(&lock);
        usleep(delay*1000); // Adjusted to ms
        printf("%lf\n", pi);
    }
    return NULL;
}


// Each thread will run mc sim i times
// N threads
// another thread N+1 that will usleep for delay*1000
// argument will be i
//

int main(int argc, char** argv){
    time_t start = time(NULL);
    srand(start);  // Seed random
    if(argc!=4){    // Check user input...
        printf("Usage ./hw13 iterations N delay\n");
        return -1;
    }

    char *endptr;
    unsigned long int iterations = strtoul(argv[1], &endptr, 0);
    if(iterations < 0){
        printf("Enter positive integer for iterations!\n");
        return -1;
    }

    int N = atoi(argv[2]);
    if(N < 1){
        printf("Enter positive integer for N!\n");
        return -1;
    }
    
    int delay = atoi(argv[3]);
    if(delay < 0){
        printf("Enter positive integer for delay!\n");
        return -1;
    }
    
    pthread_t tid[N+1]; // Create print thread
    pthread_create(&tid[N], NULL, print_pi, (void*) delay);
    
    // Create pi threads
    for(int i=0; i<N; i++){
        pthread_create(&tid[i], NULL, monty, (void*)iterations);
    }

    // Wait for pi threads
    for(int i=0; i<N; i++){
        pthread_join(tid[i], NULL);
    }
    running = 0; // Tell print thread it can exit
    pthread_join(tid[N], NULL);

    int elapsed = time(NULL) - start;

    printf("Hits: %d, Attempts: %d\nPi ~= %lf in %d seconds\n", circle_points, square_points, pi, elapsed);    // Inform user of results 

    return 0;
}






