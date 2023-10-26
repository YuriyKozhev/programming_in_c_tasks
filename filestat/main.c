#include <stdio.h> // printf
#include <unistd.h> // getcwd
#include <stdlib.h> // free
#include <dirent.h> // opendir
#include <sys/stat.h> // stat
#include <string.h> // strcmp
#include <sys/types.h> // mode_t

// . filestat.
// Напишите программу, выводящую в стандартный поток вывода 
// информацию о количестве файлов каждого типа в текущей директории.
// Максмальный балл - 5.

#define MAX_FILE_TYPE_NAME 128
#define FILE_TYPES_NUMBER 7
#define CURRENT_DIR_NAME_SIZE 2
#define MAX_FILE_NAME_SIZE 256


static char file_type_names[][MAX_FILE_TYPE_NAME] = {
    "regular files", 
    "directories", 
    "character device files", 
    "block device files", 
    "symbolic (soft) links", 
    "socket files", 
    "named pipes (FIFO)"
};

// regular files = S_ISREG
// directories = S_ISDIR
// (special) character device files = S_ISCHR
// (special) block device files = S_ISBLK
// (special) symbolic (soft) links  = S_ISLNK
// (special) socket files = S_ISSOCK
// (special) named pipes (FIFO) = S_ISFIFO

static unsigned int get_file_counts_index(mode_t st_mode) {
    if (S_ISDIR(st_mode))
        return 1;
    if (S_ISCHR(st_mode))
        return 2;
    if (S_ISBLK(st_mode))
        return 3;
    if (S_ISLNK(st_mode))
        return 4;
    if (S_ISSOCK(st_mode))
        return 5;
    if (S_ISFIFO(st_mode))
        return 6;
    if (S_ISREG(st_mode))
        return 0;
    return -1;
}

static char *contruct_abs_file_name(const char *cwd_name, const char *file_name) {
    char *abs_file_name = (char *) malloc ((CURRENT_DIR_NAME_SIZE + MAX_FILE_NAME_SIZE + 1) * sizeof (char));
    strcpy(abs_file_name, cwd_name);
    strcat(abs_file_name, "/");
    strcat(abs_file_name, file_name);
    return abs_file_name;
}


int main(int argc, char** argv) {
    int i = 0;
    int counts[] = {0, 0, 0, 0, 0, 0, 0};
    char cwd_name[CURRENT_DIR_NAME_SIZE] = ".";
    DIR* cwd;
    struct dirent *file;
    char *file_name;
    char *abs_file_name;
    struct stat file_stat;
    unsigned int file_counts_index;
    int stat_status;
    
    cwd = opendir(cwd_name);
    if (!cwd)
        return 1;

    while (file = readdir(cwd)) {
        file_name = file->d_name;
        if ((strcmp(file_name, ".") == 0) || (strcmp(file_name, "..") == 0)) // special files
            continue;
        
        abs_file_name = contruct_abs_file_name(cwd_name, file_name);
        if (!abs_file_name)
            return 1;
            
        stat_status = lstat(abs_file_name, &file_stat);
        free(abs_file_name);
        if (stat_status == -1)
            return 1;
        
        file_counts_index = get_file_counts_index(file_stat.st_mode);
        if (file_counts_index == -1)
            return 1;
        // printf("%s %s\n", file_name, file_type_names[file_counts_index]);
        counts[file_counts_index]++;
    }

    for (i = 0; i < FILE_TYPES_NUMBER; i++) {
        printf("Found %d %s\n", counts[i], file_type_names[i]);
    }

    return 0;
}