#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <utime.h>
#include <fcntl.h>
#include <ctype.h>

int check_str(const char* input) {
    for (int i = 0; i < strlen(input); i++) {
        if (!isdigit((unsigned char)input[i])) {
            return 0;
        }
    }
    return 1;
}

int bad_usage() {
    printf("Error... Usage: ./hw06 file.txt mon day year hour min sec\n");
    return -1;
}

int bad_date() {
    printf("Error: Check your date entered...\n");
    return -1;
}

int get_date(int argc, char** argv, struct tm* new_time, int fd){
    if (argc != 8) {
        return bad_usage();
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Error opening file...");
        return -1;
    }

    // Month
    if (check_str(argv[2])) {
        long month = strtol(argv[2], NULL, 10);
        if (month >= 1 && month <= 12) {
            new_time->tm_mon = month - 1;
        } else {
            return bad_date();
        }
    } else {
        return bad_date();
    }

    // Day
    if (check_str(argv[3])) {
        long day = strtol(argv[3], NULL, 10);
        if (day >= 1 && day <= 31) {
            new_time->tm_mday = day;
        } else {
            return bad_date();
        }
    } else {
        return bad_date();
    }

    // Year
    if (check_str(argv[4])) {
        new_time->tm_year = strtol(argv[4], NULL, 10) - 1900;
    } else {
        return bad_date();
    }

    // Hour
    if (check_str(argv[5])) {
        long hour = strtol(argv[5], NULL, 10);
        if (hour >= 0 && hour <= 23) {
            new_time->tm_hour = hour;
        } else {
            return bad_date();
        }
    } else {
        return bad_date();
    }

    // Minute
    if (check_str(argv[6])) {
        long minute = strtol(argv[6], NULL, 10);
        if (minute >= 0 && minute <= 59) {
            new_time->tm_min = minute;
        } else {
            return bad_date();
        }
    } else {
        return bad_date();
    }

    // Second
    if (check_str(argv[7])) {
        long second = strtol(argv[7], NULL, 10);
        if (second >= 0 && second <= 59) {
            new_time->tm_sec = second;
        } else {
            return bad_date();
        }
    } else {
        return bad_date();
    }

    if (close(fd) == -1) {
        perror("Error closing file: ");
        return -1;
    }
    return 0; 
}

int main(int argc, char* argv[]) {
    struct tm new_time;  // Corrected to struct tm instead of a pointer
    struct utimbuf mytimbuf;

    
    int fd;

    if(get_date(argc, argv, &new_time, fd) == -1){
        return -1;
    }
    
    printf("Changing file %s timestamps to %s\n", argv[1], asctime(&new_time));

    time_t epoc_seconds = mktime(&new_time);

    mytimbuf.actime = epoc_seconds;
    mytimbuf.modtime = epoc_seconds;

    utime(argv[1], &mytimbuf);

    return 0;
}


















