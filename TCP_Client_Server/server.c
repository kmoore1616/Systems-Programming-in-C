#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int sockfd;     // Kept sockfd global so all threads can access it

#define MAX_CLIENTS 100
#define BUFF_SIZE 500
int run = 1;    // Tells parent thread to keep accepting new clients

// From manpage for accept, hangup signal interupts accept. Modify its behaviour to just clear run
void exit_handle(int sig){
    run=0;
}

// Capitalizes string passed by reference
// Input: string of size 500, passed by reference
// (Supposedly) returns 0 on success -1 on failure
int to_caps(char string[500]){
    char temp_string[BUFF_SIZE];
    char token = -1;
    int i = 0;

    memcpy(temp_string, string, BUFF_SIZE);

    while(token != '\0'){
        token=temp_string[i];
        if(token >96 && token < 123){
            token-=32;
        }
        temp_string[i]=token;
        i++;
    }
    memcpy(string, temp_string, BUFF_SIZE);
    return 0;
}

// Starts a tcp server ** DOES NOT ACCEPT NEW CLIENTS **
// Input: port to open
// Output: socket file descriptor on success, -1 on failure 
int start_server(int port){
    int sockfd;
    struct sockaddr_in my_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd==-1){
        perror("Error on socket...");
        return -1;
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);

    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr))==-1){
        perror("Error on bind...");
        return -1;
    }

    if(listen(sockfd,MAX_CLIENTS) == -1){
        perror("Error on listen...");
        return -1;
    }

    return sockfd;
}

// Accepts new clients
// Input: Socket File descriptior
// Output: File descriptor of client connected, -1 on failure
int my_listen(int sockfd){ 
    int clientfd;
    struct sockaddr_in con_addr;
    socklen_t struct_size;
    struct_size = sizeof(con_addr);
    clientfd = accept(sockfd, (struct sockaddr*)&con_addr, &struct_size);   // Main thread will halt here while waiting for new clients, interrupted by any signal
    if(clientfd == -1){
        if(run ==1){    // If run is cleared, that means a signal was sent and the accept call failed with EINTR and we can ignore the error.
            perror("Error on accept...");
            return -1;
        }
        return 0;
    }
    printf("Connection Established...\n");
    return clientfd;
}

// Thread handler for each client
void* client(void* arg){
    int clientfd = (int)arg;
    char buffer[BUFF_SIZE];
    int bytes_read;
    while(run){ // Constantly read in user input from cleints
        memset(buffer, 0, BUFF_SIZE);   // Clear buffer for next read
        bytes_read = read(clientfd, buffer, BUFF_SIZE); // Wait for next read
        if(bytes_read == -1){
            perror("Error reading from client");
            exit(-1);
        }
        buffer[bytes_read] = '\0';
        buffer[strcspn(buffer, "\r\n")] = 0;    // Clear junk chars recieved (Only nececarry for telnet?)
        printf("Message from client:%d is %s\n",clientfd, buffer); 
        if(strcmp(buffer, "quit") == 0){    // If quit recieved
            kill(getpid(), SIGHUP); // Send SIGHUP signal to terminate the accept call main thread is waiting in and break out of fx
            break;
        }else{
            to_caps(buffer);    // Otherwise capitalize and send back to client
            if(write(clientfd, buffer, BUFF_SIZE)==-1){
                perror("Error writing to client...");
                exit(-1);
            }
        }

    }         
    return NULL;
}

int main(int argc, char** argv){
    int clientfd;
    int port;
    int i = 0;
    pthread_t tid[MAX_CLIENTS]; 

    // Check user input
    if(argc!=2){
        printf("Usage: ./server PORT_NUMBER\n");
        return -1;
    }

    port = atoi(argv[1]);
    
    sockfd = start_server(port); // Open server
    if(sockfd==-1){
        printf("Error starting server...\n");
        return -1;
    }    
    
    // Regular signal api cant clear SA_RESTART flag, so EINTR is never raised and accept never fails. 
    struct sigaction sa;
    sa.sa_handler = exit_handle;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;  // <== disables SA_RESTART 
    sigaction(SIGHUP, &sa, NULL);   // Basically Signal API mapping SIGHUP to exit_handle

    while(run){
        clientfd = my_listen(sockfd);   // Halts main thread until new client connected
        if(clientfd == -1){
            printf("Error connecting client...\n");
            return -1;
        }else if(clientfd == 0){    // 0 Signifies exit recieved, dont make new thread
            break;
        }
        if(pthread_create(&tid[i], NULL, client, (void*) clientfd)){    // If new client connected spin up new thread to handle it
            perror("Error spinning up thread...");
            return -1;
        }

        i++;     
    }

    if(close(sockfd)==-1)
        perror("Error... Close Sockfd");

    if(close(clientfd)==-1)
        perror("Error... Close Clientfd");
    return 0;
}

