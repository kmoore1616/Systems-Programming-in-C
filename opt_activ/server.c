#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int sockfd;     // Kept sockfd global so all threads can access it

#define MAX_CLIENTS 100
#define BUFF_SIZE 500
int run = 1;    // Tells parent thread to keep accepting new clients

// From manpage for accept, hangup signal interupts accept. Modify its behaviour to just clear run
void exit_handle(int sig){
    run=0;
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


int process_args(char* u_in, char** arg_vars, int arg_count){
    char* token;
    int i = 0;
    token = strtok(u_in, " ");  // Grab first token
    if(token == 0x0){   // If its 0x0 (nothing was passed) ignore command
        return -1;
    }

    while(token != NULL){   // Loop through string and tokenize
        arg_vars[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    arg_vars[arg_count] = NULL;
    return 0;
}

int handle_exec(char buffer[BUFF_SIZE], char out_buff[BUFF_SIZE]){
    int wc = count_words(buffer);
    char** arg_vars = (char**) malloc(sizeof(char*)*wc);
    
    if(process_args(buffer, arg_vars, wc)==-1){
        printf("Error proc args...\n");
        exit(-1);
    }
    
    int pfd[2];

    pipe(pfd);
    int pid = fork();
    // Pipe stdout to pfd[w] read -> buffer send buffer
    // buffer = read(pfd[w])
    
    if(pid==0){
        close(pfd[0]);  // Close read
        dup2(pfd[1], 1);
        dup2(pfd[1], 2);    // Just fucking throw stderr onto the pipe too
        execvp(arg_vars[0],arg_vars);
        // Send stderr anyways
        perror("");
        exit(0);
    }
    close(pfd[1]);
    if(read(pfd[0], out_buff, BUFF_SIZE) == -1){
        perror("Read from pipe...");
        return -1;
    }

    close(pfd[0]);
    wait(NULL);
    return 0;
}


// Thread handler for each client
void* client(void* arg){
    int clientfd = (int)arg;
    char buffer[BUFF_SIZE];
    int bytes_read;
    char out_buff[BUFF_SIZE];
    while(run){ // Constantly read in user input from cleints
        memset(buffer, 0, BUFF_SIZE);   // Clear buffer for next read
        bytes_read = read(clientfd, buffer, BUFF_SIZE); // Wait for next read
        if(bytes_read == -1){
            perror("Error reading from client");
            exit(-1);
        }
        buffer[bytes_read] = '\0';
        if(strcmp(buffer, "quit\n") == 0){    // If quit recieved
            kill(getpid(), SIGHUP); // Send SIGHUP signal to terminate the accept call main thread is waiting in and break out of fx
            break;
        }else{
            if(handle_exec(buffer, out_buff) ==-1){
                printf("Error on handle exec...\n");
                exit(-1);
            }
            if(write(clientfd, out_buff, BUFF_SIZE)==-1){
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

