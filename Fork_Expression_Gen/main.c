#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>


void usage_err(void){
    printf("Error... Usage ./hw06 text.txt 10\n");  // Error if bad input
    exit(-1);
}

int rand_base(int base){
    int rand_num = rand() % base; // Returns a random number of base x.
    return rand_num;
}

int int_to_op(int op, char* out){ // Given an int base 5, returns the appropriate operator
    if(out == NULL){
        return -1;
    }

   switch (op) {
        case 0:
            *out = '+';
            break;
        case 1:
            *out = '-';
            break;
        case 2:
            *out = '*';
            break;
        case 3:
            *out = '/';
            break;
        case 4:
            *out = '%';
            break;
        default:
            return -1;
   }
   return 0;

}

int create_expressions(char* f_name, char* num){    // Function the child uses to write expressions to file
    srand(time(NULL) ^ getpid());   // Seed random number
    int num_expr = atoi(num);   // Total number requested by user
    int x;
    int y;
    char op;    // Character of operator
    int fd;     // File descriptor of file to be created
    int res;    // Result of various operations
    char to_write[4];   // Character array that holds the next expression to be written to file
    fd = open(f_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);  // Open file
    if(fd == -1){   // If failed inform user and exit program
        perror("Error...");
        return -1;
    }


    for(int i=0; i<num_expr; i++){  // Loop to write x expressions to file
        x = rand_base(10);  // Grab rand_num 0-9
        y = rand_base(10);  // Again
        if(int_to_op(rand_base(5), &op)==-1){   // Grab operator
            printf("Operator fetch failed...\n");   // Inform user and exit if failed
            return -1;  // If
        }
        sprintf(to_write, "%d%c%d", x, op, y);  // Format expression
        
        res = write(fd, to_write, 3);   // Write formatted expression
        if(res == -1){
            perror("Error..."); // Inform user and exit if failed
            return -1;
        }
    }
    close(fd);  // Cleanup
    return 0;   // Return success
}




int compute(char* f_name, char* num){ // Funct used by parent to evaluate expressions in file after child exits
    int fd;
    int f_len = atoi(num);
    int res;
    int x,y;
    char op;

    char f_contents[f_len*3]; // Array to hold contents of created file
    
    fd = open(f_name, O_RDONLY);    // Open file
    if(fd==-1){
        perror("Error..."); // Inform user and exit if fails
        return -1;
    }

    res = read(fd, f_contents, f_len*3);    // Read file
    if(res == -1){  // Inform user and exit if fails
        return -1;
    }

    for(int i=0; i<f_len*3; i+=3){  // Loop through each expression in file
        x = (int)f_contents[i]-48;  // Grab x (adjusted for ascii)
        op = f_contents[i+1];   // Grab op
        y = (int)f_contents[i+2]-48; // Grap y (adjusted for ascii)


        switch (op) {   // Select correct operation and print out result...
            case '+':
                res = x + y;
                printf("%d %c %d = %d\n", x, op, y, res);
                break;

            case '-':
                res = x - y;
                printf("%d %c %d = %d\n", x, op, y, res);
                break;

            case '*':
                res = x * y;
                printf("%d %c %d = %d\n", x, op, y, res);
                break;

            case '/':
                res = (y == 0) ? -1 : x / y;  // -1 if divide by zero
                printf("%d %c %d = %d\n", x, op, y, res);
                break;

            case '%':
                res = (y == 0) ? -1 : x % y;  // -1 if mod zero
                printf("%d %c %d = %d\n", x, op, y, res);
                break;

            default:
                return -1;
        }

    }
    // Cleanup
    close(fd);
    return 0;
}



int main(int argc, char** argv){
    bool is_parent;
    if(argc !=3){   // User should provide 2 arguments
        usage_err();
    }
    char *ext = argv[1] + strlen(argv[1])-4;    // Grab file extension
    if(strcmp(".txt", ext)!=0){ // If it isnt .txt throw error
        usage_err();
    }
    
    if(atoi(argv[2]) < 1){  // If < 1 or not a digit, throw error
        usage_err();
    }

    
    is_parent = (fork() != 0) ? true : false;   // Fork and determine if process is parent or child

    if(is_parent){  // If is parent
        wait(NULL); // Wait for child
        if(compute(argv[1], argv[2])==-1){  // And compute the values
            printf("Error, something went wrong...\n"); // Inform user of error
            exit(-1);
        }
    }else{ // If is child
        if(create_expressions(argv[1], argv[2]) == -1){ // Create expression file
            printf("Error, something went wrong...\n"); // Inform user of error
            exit(-1);
        }
    }
    return 0;
}

