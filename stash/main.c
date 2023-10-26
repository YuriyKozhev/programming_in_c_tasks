#include <stdio.h> // printf
#include <sys/stat.h> // mkdir
#include <fcntl.h> // open
#include <string.h> // strcat
#include <stdlib.h> // malloc
#include <unistd.h> // read, write

// . stash. 
// Напишите программу, скрывающую тип файла с целью обеспечить невозможность его 
// чтения стандартными средствами. 
// Программа должна иметь 2 режима работы - искажение и восстановление. 
// Режим работы и имя файла передайте с помощью аргументов командной строки.
// Максмальный балл - 10.


#define RESTORE_MODE "restore"
#define DISTORT_MODE "distort"
#define DISTORT_MESSAGE "di"


off_t get_file_size(int fd) {
    struct stat st;
    off_t size;
    fstat(fd, &st);
    size = st.st_size;
    return size;
}


void inspect_file(char *filename) {
    int filename_fd = open(filename, O_RDONLY, S_IRUSR);
    ssize_t file_size_read;
    off_t file_size = get_file_size(filename_fd);
    char *buf = (char *) malloc(file_size * sizeof(char) + 1);
    file_size_read = read(filename_fd, buf, file_size);
    printf("%ld, first 5: %c %c %c %c, last 5: %c %c %c %c\n", file_size_read,
        buf[0], buf[1], buf[2], buf[3], 
        buf[file_size - 4], buf[file_size - 3], buf[file_size - 2], buf[file_size - 1]
    );
    close(filename_fd);
}


int main(int argc, char **argv) {
    char *filename = argv[1];
    char *mode = argv[2];
    int filename_fd;
    off_t offset;
    off_t file_size;
    char *buf;    
    ssize_t file_size_read;
    ssize_t file_size_written;
    printf("Processing file: %s, with mode: %s\n", filename, mode);
    inspect_file(filename);
    if (strcmp(mode, RESTORE_MODE) == 0) {
        filename_fd = open(filename, O_RDONLY, S_IWUSR);
        offset = lseek(filename_fd, strlen(DISTORT_MESSAGE), SEEK_SET);
        file_size = get_file_size(filename_fd);
        buf = (char *) malloc(file_size * sizeof(char) + 1);
        file_size_read = pread(filename_fd, buf, file_size, offset);
        close(filename_fd);
        
        filename_fd = open(filename, O_WRONLY | O_TRUNC, S_IWUSR);
        file_size_written = pwrite(filename_fd, buf, file_size_read, SEEK_SET);
        close(filename_fd);
    } else if (strcmp(mode, DISTORT_MODE) == 0) {
        filename_fd = open(filename, O_RDONLY, S_IRUSR);
        file_size = get_file_size(filename_fd);
        buf = (char *) malloc(file_size * sizeof(char) + 1);
        file_size_read = read(filename_fd, buf, file_size);
        close(filename_fd);

        filename_fd = open(filename, O_WRONLY | O_TRUNC, S_IWUSR);
        file_size_written = write(filename_fd, DISTORT_MESSAGE, strlen(DISTORT_MESSAGE));
        file_size_written = write(filename_fd, buf, file_size_read);
        close(filename_fd);
    } else {
        printf("Incorrect mode: %s\n", mode);
        return 1;
    }
    inspect_file(filename);
    return 0;
}
