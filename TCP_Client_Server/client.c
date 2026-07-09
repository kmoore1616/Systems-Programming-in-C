#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>


int main(int argc, char** argv){
    char buffer[500];
    char in_buffer[500];
    // Check user input
    if(argc!=2){
        printf("Usage: ./client DESTPORT\n");
        return -1;
    }
    
    int port = atoi(argv[1]);

    struct sockaddr_in dest_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        perror("Error on socket open");
        return -1;
    }

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(sockfd,(struct sockaddr*)&dest_addr, sizeof(struct sockaddr))==-1){
        perror("Error on connect...");
        return -1;
    }
    int run =1; 
    while(run){
        memset(buffer, 0, 500); // Reset input and write buffers
        memset(in_buffer, 0, 500);
        printf("Send Message:");
        fgets(buffer, 500, stdin);
        buffer[strcspn(buffer, "\n")] = 0;    // Clear junk chars recieved 
        if(write(sockfd, buffer, 500)==-1){ 
            perror("Error writing to server...");
            return -1;
        }
        if(strcmp(buffer, "quit")==0){
            printf("Terminating Program...\n");
            break;
        }
        if(read(sockfd, in_buffer, 500)==-1){
            perror("Error reading from server...");
            return -1;
        }
        printf("Recieved Message: %s\n", in_buffer);
    }
    if(close(sockfd)== -1)
        perror("Error... close sockfd");
    return 0;
}

