/*
Need to read in user input to determine what to do

*Note .mtar file format:
-------------------------------------------------------------------------------------------------
HEADER denoted as 01        //(header_start)
loop through stat structure to save all metadata sequentially as it appears in man pages
02                          //(end_header)
Loop through buffer to place all read data into the file
03						   //(EOF)
01                         //(Start of next file header)
*Header*
02
*DATA*
03 						   
...
03
04						   //(End of transmission/file)	
--------------------------------------------------------------------------------------------------
Archive:
A. Open new .mtar file
b. for loop to archive each file
	0. Print SOH to signify start of header
	1. Read in metadata and write to header exactly as seen in man
	2. Print SOTX to signify end of header and start of data
	3. Read in file data into mtar
	4. Print EOT&SOH to signify end of data and start of next header
c. end of for loop print EOTM and close file and free memory

*/


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <utime.h>

#define XTRACT 0	// Int to hold what mode is being used
#define ARCHIV 1


#define READ_NAME 1
#define READ_MODE 2
#define READ_UID 3
#define READ_GID 4
#define READ_ACTIME 5
#define READ_MOD_TIME 6
#define READ_DAT 7


int i;
int arch_flags = O_RDWR | O_CREAT | O_TRUNC;		// Write-only and create new file
int permis = S_IRWXU | S_IRWXG | S_IRWXO;		// Full user control (lazy)
int ex_flags = O_RDONLY;
int result;

char SOH = '\x9F';		// Start of header
char SOT = '\xC2';		// Start of text
char EOTX = '\xA9';		// End of text
char EOTR = '\xEF';		// End of transmission(file)

	


/*
Needs to open .mtar file user puts in.
while loop until eotr(file end)
	collect data & metadata
write to new files

Formatted as:
\01name.txt|mode|uid|gid|actime|mtime|\02**data**\03\01**Header** or \04 for end of text
Need a few variables: -- all length of file_cnt
char* f_names -> used to hold file names
int* f_modes -> used to hold file modes 
int* f_uids -> used to hold file uids 
int* f_gids -> used to hold file gids 
int* f_actimes -> used to hold file access times
int* f_modtimes -> used to hold file modify times

Step 1. Open .mtar file X

Step 2. Find how many files are in .mtar (count SOH's until EOTR) X

Step 3. Initilaize all arrays X

Step R4. Gather data into arrays X

Step 4. Open files to be written

Step 5. Assign data to files

Step 6. Cleanup

Step 7. (:
*/
int extract(char* file_in){
    char** f_names;
    int* f_modes;
    int* f_uids;
    int* f_gids;
    int* f_actimes;
    int* f_modtimes; 
    int* file_desr;
    int* file_sizes;
    char* read_buf;
    char** f_contents;

    char* mt_content;
    struct stat* mt_stats =(struct stat*) malloc(sizeof(struct stat));

    int mt_size; // Holds size in bytes of .mtar file passed
    int file_size = 0;
    int file_cnt = 0;

    int mt_fd = result = open(file_in, ex_flags); // Open .mtar file
    if(mt_fd == -1){
        return -1;
    }
    
    result = fstat(mt_fd, mt_stats);
    if(result == -1){
        return -1;
    }
    mt_size = mt_stats->st_size;    // Grab .mtar size
    
    mt_content = (char*) malloc(sizeof(char)*mt_size); 
    read_buf = (char*) malloc(sizeof(char)*mt_size);
    f_contents = (char**) malloc(sizeof(char*)*mt_size);

    result = read(mt_fd, mt_content, mt_size);
    if(result == -1){
        return -1;
    }

    for(int i=0; i<mt_size; i++){
        if(mt_content[i] == SOH){
            file_cnt++;
            printf("%x\n", mt_content[i]);
        }
    }
    
    f_names = (char**) malloc(sizeof(char*)*file_cnt);
    f_modes = (int*) malloc(sizeof(int)*file_cnt);
    f_uids = (int*) malloc(sizeof(int)*file_cnt); 
    f_gids = (int*) malloc(sizeof(int)*file_cnt); 
    f_actimes = (int*) malloc(sizeof(int)*file_cnt); 
    f_modtimes = (int*) malloc(sizeof(int)*file_cnt); 
    file_desr = (int*) malloc(sizeof(int)*file_cnt); 
    file_sizes = (int*) malloc(sizeof(int*)*file_cnt);

    int buf_counter = 1;
    int temp = 0;   // Used to determine random file sizes

  
    // Used to determine what part of the file is being read in
    int read_part = READ_NAME;

    for(int file=0; file<file_cnt; file++){  // Loop through all files
        while(mt_content[buf_counter] != SOH && mt_content[buf_counter] != EOTR){  // Until next file
            switch(read_part){ // Determine what part of the file is being read in
                case READ_NAME: // Are you reading file name?
                    memset(read_buf, 0, mt_size);
                    while(mt_content[buf_counter] != '|'){  // Yes? Grab it and save to f_names[file]
                        read_buf[temp] = mt_content[buf_counter]; 
                        temp++;
                        buf_counter++;
                    } 
                    buf_counter++;
                    char* file_name = (char*)malloc(sizeof(char)*temp);
                    memcpy(file_name, read_buf, temp);
                    f_names[file] = file_name;
                    //printf("Name: %s", f_names[file]);
                    read_part = READ_MODE;
                    temp = 0;

                    break;
                case READ_MODE: // Are you reading file mode?
                    memset(read_buf, 0, mt_size);   // Yes? Grab it and save to f_modes
                    while(mt_content[buf_counter] != '|'){  // Yes? Grab it and save to f_names[file]
                        read_buf[temp] = mt_content[buf_counter]; 
                        temp++;
                        buf_counter++;
                    } 
                    buf_counter++;
                    f_modes[file] = atoi(read_buf);
                    //printf("Mode: %d", f_modes[file]);
                    read_part = READ_UID;
                    temp = 0;
                    
                    break;

                case READ_UID:
                    memset(read_buf, 0, mt_size);   // Yes? Grab it and save to f_modes
                    while(mt_content[buf_counter] != '|'){  // Yes? Grab it and save to f_names[file]
                        read_buf[temp] = mt_content[buf_counter]; 
                        temp++;
                        buf_counter++;
                    } 
                    buf_counter++;
                    f_uids[file] = atoi(read_buf);
                    //printf("UID: %d", f_uids[file]);
                    read_part = READ_GID;
                    temp = 0;
                    break;

                case READ_GID:

                    memset(read_buf, 0, mt_size);   // Yes? Grab it and save to f_modes
                    while(mt_content[buf_counter] != '|'){  // Yes? Grab it and save to f_names[file]
                        read_buf[temp] = mt_content[buf_counter]; 
                        temp++;
                        buf_counter++;
                    } 
                    buf_counter++;
                    f_gids[file] = atoi(read_buf);
                    //printf("GID: %d", f_gids[file]);
                    read_part = READ_ACTIME;
                    temp = 0;

                    break;
                case READ_ACTIME:
    
                    memset(read_buf, 0, mt_size);   // Yes? Grab it and save to f_modes
                    while(mt_content[buf_counter] != '|'){  // Yes? Grab it and save to f_names[file]
                        read_buf[temp] = mt_content[buf_counter]; 
                        temp++;
                        buf_counter++;
                    } 
                    buf_counter++;
                    f_actimes[file] = atoi(read_buf);
                    //printf("Acc time: %d", f_actimes[file]);
                    read_part = READ_MOD_TIME;
                    temp = 0;
                    break;

                case READ_MOD_TIME:
                    memset(read_buf, 0, mt_size);   // Yes? Grab it and save to f_modes
                    while(mt_content[buf_counter] != '|'){  // Yes? Grab it and save to f_names[file]
                        read_buf[temp] = mt_content[buf_counter]; 
                        temp++;
                        buf_counter++;
                    } 
                    buf_counter++;
                    f_modtimes[file] = atoi(read_buf);
                    //printf("Modify: %d", f_modtimes[file]);
                    read_part = READ_DAT;
                    temp = 0;
                    
                    break;
                
                case READ_DAT:
                    buf_counter++;
                    memset(read_buf, 0, mt_size);
                    while(mt_content[buf_counter] != EOTX){
                        read_buf[temp] = mt_content[buf_counter];
                        temp++;
                        buf_counter++;
                        file_size++;
                    }
                    buf_counter++;
                    char* file_contents = (char*) malloc(sizeof(char)*file_size);
                    memcpy(file_contents, read_buf, file_size);
                    f_contents[file] = file_contents;
                    file_sizes[file] = file_size;
                    //printf("File Contents: %s", f_contents[file]);
                    file_size = 0;
                    temp = 0;
                    read_part = READ_NAME;
                    break;


                default:
                    printf("Case statement shouldnt get here lol");
                    errno = 11;
                    return -1;
            }
        }
        buf_counter++;
    }

    for(int i=0; i<file_cnt; i++){
        struct utimbuf time;
        file_desr[i] = open(f_names[i], O_RDWR | O_CREAT);
        if(file_desr[i] == -1){
            return -1;
        }
        
        result = write(file_desr[i], f_contents[i], file_sizes[i]);
        if(result == -1){
            return -1;
        }
        
        result = fchown(file_desr[i], f_uids[i], f_gids[i]);
        if(result == -1){
            return -1;
        }
        
        time.actime = f_actimes[i];
        time.modtime = f_modtimes[i];
        
        result = utime(f_names[i], &time);
        if(result == -1){
            return -1;
        }

        result = fchmod(file_desr[i], f_modes[i]);
        if(result == -1){
            return -1;
        }
        

        close(file_desr[i]);  // Close file descriptor
        free(f_names[i]);      // Free allocated filename
        free(f_contents[i]);   // Free allocated file content

    }
    free(f_names);
    free(f_modes);
    free(f_uids);
    free(f_gids);
    free(f_actimes);
    free(f_modtimes);
    free(file_desr);
    free(file_sizes);
    free(read_buf);
    free(f_contents);
    free(mt_content);
    free(mt_stats);
    

    return 0;
}



/* Archive
Input: i_files from argv (file names), file_cnt from argc.
Output: -1 if failure 0 otherwise
Description: Writes a new .mtar following the outline described above
*/
int archive(char* i_files[], int file_cnt){
	int* file_des = (int*) malloc(sizeof(int)*file_cnt);	// Array of all file descriptors of files to be opened
	int* file_sizes = (int*) malloc(sizeof(int)*file_cnt);	// Array of all file lens
	struct stat** file_stats = (struct stat**) malloc(sizeof(struct stat*)*file_cnt);
	char** f_dat = (char**)malloc(sizeof(char*)*file_cnt);	// Array to hold data from each file
	char *buf; 												// Buffer to hold data read in from file
    char* m_header;
	char meta_buf[100];										// String to write to mdata
	char temp[20];											// To convert int to str

	int fd;					// Temp variable to store in file_des
	int bytes_to_read;			// Bytes read in to each file
	int meta_size = 0;
	ssize_t wr_res;		// Write result for error checking

	// Loop through file names and open
	for(i=3; i<file_cnt+3; i++){
		fd = open(i_files[i], O_RDONLY, S_IRWXU);
		if(fd == -1){	// If failure
			printf("%s\n", i_files[i]);		// Inform user of problematic file
			return -1;		// And return failure
		}
		file_des[i-3] = fd;	// Otherwise add opened file to file_des.
	}

	// Loop through opened files to save both data and metadata
	for(i=0; i<file_cnt; i++){
		file_stats[i] = (struct stat*)malloc(sizeof(struct stat));	// Add a new stat struct to stat array
		result = fstat(file_des[i], file_stats[i]);			// Grab file stats
		if(result == -1){
			return -1;
		}
		bytes_to_read = file_stats[i]->st_size;		// Find how much is in each file
		file_sizes[i] = bytes_to_read;
		buf = (char*) malloc(sizeof(char)*bytes_to_read);	// Allocate string to fit data
		result = read(file_des[i], buf, bytes_to_read);		// Read in data
		f_dat[i] = buf;		// Append to file data
	}

	// Create and populate .mtar file
	int tfd = open(i_files[2], arch_flags, permis);	// Create and open mtar file
	struct utimbuf times;

	// File name (find . then grab next 3), 
	// mode (?bytes) uid(?bytes) gid(?bytes), actime(?bytes), modtime(?bytes) and then sot 
	for(i=0; i<file_cnt; i++){

		memset(meta_buf, 0, 100);								// Clear array
        sprintf(temp, "%s|", i_files[i+3]);     // Grab file name & add |
        strcat(meta_buf, temp); // Append to meta buffer
		meta_size += strlen(temp);  // Increment meta_size
        times.actime = file_stats[i]->st_atime;
		times.modtime = file_stats[i]->st_mtime;
		
        sprintf(temp, "%d|", file_sizes[i]);
        meta_size += strlen(temp);
        strcat(meta_buf, temp);

		sprintf(temp, "%d|", file_stats[i]->st_mode);   // Grab mode
		meta_size += strlen(temp);
		strcat(meta_buf, temp);
		
        sprintf(temp, "%d|", file_stats[i]->st_uid);    // Grab uid
		meta_size += strlen(temp);
		strcat(meta_buf, temp);
		
        sprintf(temp, "%d|", file_stats[i]->st_gid);    // Grab gid
		meta_size += strlen(temp);
		strcat(meta_buf, temp);
        
        sprintf(temp, "%ld|", times.actime);            // Grab access time
		meta_size += strlen(temp);
		strcat(meta_buf, temp);
		
        sprintf(temp, "%ld|", times.modtime);           // Grab mod time
		meta_size += strlen(temp);
		strcat(meta_buf, temp);
        
		
		
		if(write(tfd, &SOH, 1) == -1){		// Write start of header
			return -1;
		}
		if(write(tfd, meta_buf, meta_size) == -1){	// Write header
			return -1;
		}
		if(write(tfd, &SOT, 1) == -1){		// Write start of text
			return -1;
		}
		if(write(tfd, f_dat[i], file_sizes[i]) == -1){		// Write data
			return -1;
		}
		if(write(tfd, &EOTX, 1) == -1){		// Write end of text
			return -1;
		}
		if(i == file_cnt-1){
			if(write(tfd, &EOTR, 1) == -1){	// Write end of transmission(file){
				return -1;
			}
		}
		meta_size = 0;
	}
	
	

	for(i=0; i<file_cnt; i++){
		close(file_des[i]);
		free(file_stats[i]);
		free(f_dat[i]);
	}
	close(tfd);
	free(file_stats);
	free(f_dat);
}

int main(int argc, char* argv[]){
	int mode;
	int file_cnt = argc - 3;
	umask(0022);

	if(argc < 2){
		printf("Error... Enter in a mode and file\n");
		exit(-1);
	}else if(strcmp(argv[1], "-a") == 0){
		if(argc == 2){
			printf("Error... Please enter at least one file name\n");
			exit(-1);
		}
		char *extension = argv[2] + strlen(argv[2]) - 5;	// Grab .mtar from end of file
		if(strcmp(extension, ".mtar") != 0){
			printf("Error... usage ./myTar -a file.mtar file1.txt ...\n");
			exit(-1);
		}
		mode = ARCHIV;
	}else{
		if(argc > 3){
			printf("Error... Only include one file to extract\n");
			exit(-1);
		}
		mode = XTRACT;
	}
	
	if(mode == ARCHIV){
		result = archive(argv, file_cnt);
	}else{
		result = extract(argv[2]);
	}

	if(result == -1){
		fprintf(stderr, "Error... %s\n", strerror(errno));
	}

	
	return 0;
}









