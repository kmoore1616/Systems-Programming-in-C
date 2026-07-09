#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

// Used to determine what settings user selected ie in=input.txt -append etc...
#define none_spec 0b00000
#define in_spec 0b1000
#define out_spec 0b0100
#define err_spec 0b0010
#define apnd_spec 0b0001

int wrt_flg = O_WRONLY;

int get_user_in(char* u_in){
    int read_cnt = read(0, u_in, 1000-1);   // Read at max 999 characters (size of maxbuf)
    if(read_cnt == -1){
        return -1;
    }   
    u_in[read_cnt-1] = '\0';    // Add null terminator
    return 0;
}

int print_both(char* u_in){
    write(1, "stdout: ", 8);
    write(1, u_in, strlen(u_in));
    write(1, "\n", 1);
    
    write(2, "stderr: ", 8);
    write(2, u_in, strlen(u_in));
    write(2, "\n", 1);
    return 0;
}

// Gathers user's settings and filenames
// Input: Argc, Argv
// Returns:
//      - Retval: A 4-bit number that contains a bitmap that describes all possible combos of settings
//      - file_vec: A vector that contains pointers to strings holding files user specified
int process_args(int argc, char** argv, char *file_vec[3]){
    int retval = 0;    // 4-bit num to return
    int str_size = 0;   // Used to determine amount of mem to allocate to each file string
    
    if(argc == 1){  // If nothing is specifed return none_spec
        retval = none_spec;
        return retval;  // Return that no options were specified
    }

    for(int i=1; i<argc; i++){    // Loop through all supplied arguments
        char temp[10];
        if(strncmp(argv[i], "in=", 3)==0){ // Strncpy grabs first x chars and then compares to possible settings
            retval |= in_spec;  // If in is specifed modify retval and store file name
            str_size = strlen(argv[i]+3);
            file_vec[0] = (char*) malloc(sizeof(char)*str_size);
            strncpy(file_vec[0], argv[i]+3, str_size);
        }else if(strncmp(argv[i], "out=", 4)==0){
            retval |= out_spec;     // Out in is specifed modify retval and store file name
            str_size = strlen(argv[i]+4);
            file_vec[1] = (char*) malloc(sizeof(char)*str_size);
            strncpy(file_vec[1], argv[i]+4, str_size);
        }else if(strncmp(argv[i], "err=", 4)==0){
            retval |= err_spec;     // If err is specifed modify retval and store file name
            str_size = strlen(argv[i]+4);
            file_vec[2] = (char*) malloc(sizeof(char)*str_size);
            strncpy(file_vec[2], argv[i]+4, str_size);
        }else if(strncmp(argv[i], "-append", 7)==0){  
            retval |= apnd_spec;     // If append is specifed modify retval
        }else{
            printf("Error... Usage ./hw09 in=input.txt out=output.txt err=err.txt -append\n");    // If unknown setting is encountered warn user
            return -1;
        }
    }
    return retval;
}


int main(int argc, char** argv){
    char *file_vec[3];
    char u_in[1000];
    int retval = process_args(argc, argv, file_vec);
    int read_cnt;
    int in_fd;
    int out_fd;
    int err_fd;
    int read_size;
    if(retval == -1){
        for(int i=0; i<3; i++){
            free(file_vec[i]);
        }
        exit(-1);
    }

    if((retval & out_spec)){
        if((retval & apnd_spec)){
            out_fd = open(file_vec[1], O_WRONLY | O_APPEND | O_CREAT, S_IRWXU);
        }else{
            out_fd = open(file_vec[1], O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU);
        }
        dup2(out_fd, 1);
    }
    if((retval & err_spec)){
        if((retval & apnd_spec)){
            err_fd = open(file_vec[2], O_WRONLY | O_APPEND | O_CREAT, S_IRWXU);
        }else{
            err_fd = open(file_vec[2], O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU);
        }
        dup2(err_fd, 2);
    }


    if((retval & in_spec)){    // Inspec
        in_fd = open(file_vec[0], O_RDONLY);
        dup2(in_fd, 0);
        read_cnt = read(in_fd, u_in, 999);
        u_in[read_cnt] = '\0';
        print_both(u_in);
    }else{    // none-specified
        while(1){   // Bad
            if(get_user_in(u_in) == -1){
                perror("Err - User input...");
                for(int i=0; i<3; i++){
                    free(file_vec[i]);
                }
                exit(-1);
            }
            print_both(u_in);
        }
    }
    for(int i=0; i<3; i++){
        free(file_vec[i]);
    }

    return 0;
}



