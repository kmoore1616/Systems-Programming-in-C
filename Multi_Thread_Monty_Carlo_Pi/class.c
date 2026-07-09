#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


int balance = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* deposit(void* arg){
    int amount = (int)arg;
    pthread_mutex_lock(&lock);
    balance+=amount; 
    printf("Depositing $%d...\nNew account value: $%d\n", amount, balance);
    pthread_mutex_unlock(&lock);
    return NULL;
}



void* withdraw(void* arg){
    int amount = (int)arg;
    pthread_mutex_lock(&lock);
    if(amount > balance){
        balance-=30;
        printf("OVERDRAFT!!! $%d\nNew account value:%d\n", amount, balance);
    }else{
        balance-=amount;
        printf("Withdrawing $%d...\nNew account value: $%d\n", amount, balance);
    }
    pthread_mutex_unlock(&lock);

    return NULL;
}


int main(int argc, char** argv){
    srand(time(NULL)); 
    int random;
    int max = 500;
    int min = 100;
    pthread_t tid[10];


    for(int i=0; i<10; i++){
        random = rand() % (max - min + 1) + min;    
        if(i<5){
            pthread_create(&tid[i], NULL, deposit, (void*)random);
        }else{
            pthread_create(&tid[i], NULL, withdraw, (void*)random);
        }
    }
    for(int i=0; i<10; i++){
        pthread_join(tid[i], NULL);
    }

    printf("Ending balance: %d\n", balance);


    return 0;
}

