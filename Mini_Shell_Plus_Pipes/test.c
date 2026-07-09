
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char** argv){
    int mypipe[2];
    char mybuff[100];
    int retval = pipe(mypipe);
    retval = fork();
    if(!retval){
        //child
        close(mypipe[1]); // Close unused channel (Write)
        dup2(mypipe[0], 0); // Map stdin        
        fgets(mybuff, 100, stdin);  // Goop! (Read from pipe)
        printf("%s\n", mybuff);
        retval = execlp(mybuff, mybuff, NULL);
        
        if(retval == -1){
            perror("Error on exec... %s");
        }



    }else{
        //parent
        close(mypipe[0]);   // Close unused channel (read)
        dup2(mypipe[1], 1); // Map stdout to pipe
        printf("ls");  // Write to pipe

    }


    return 0;
}
