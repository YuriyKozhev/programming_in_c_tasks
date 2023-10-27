#include <stdio.h> // printf
#include <sys/stat.h> // mkdir
#include <fcntl.h> // open
#include <string.h> // strcat
#include <stdlib.h> // malloc
#include <unistd.h> // read, write

// . stego. 
// Напишите программу, позволяющую передать скрытое послание в файле формата jpeg. 
// Файл при этом должен продолжать читаться стандартными средствами. 
// Программа должна иметь 3 режима работы - добавление послания, чтение послания, удаление послания. 
// Режим работы, послание и имя файла передайте с помощью аргументов командной строки. 
// Максмальный балл - 10.


#define ADD_MODE "add"
#define READ_MODE "read"
#define DELETE_MODE "delete"
#define MESSAGE_END "$"
#define MAX_MESSAGE_LEN 3


off_t get_file_size(int fd) {
    struct stat st;
    off_t size;
    fstat(fd, &st);
    size = st.st_size;
    return size;
}


void inspect_file(char *filename) {
    int filename_fd = open(filename, O_RDONLY, S_IRWXU);
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
    int i = 0, j = 0;
    char *filename = argv[1];
    char *mode = argv[2];
    char *message;
    int filename_fd;
    off_t offset;
    off_t file_size;
    char *buf;
    char *padding;  
    int padding_size;    
    ssize_t file_size_read;
    ssize_t file_size_written;
    int message_end = 0;
    printf("Processing file: %s, with mode: %s\n", filename, mode);
    inspect_file(filename);
    if (strcmp(mode, ADD_MODE) == 0) {
        message = argv[3];
        printf("Adding message: %s\n", message);
        filename_fd = open(filename, O_WRONLY | O_APPEND, S_IRWXU);
        file_size_written = write(filename_fd, message, strlen(message));
        file_size_written = write(filename_fd, MESSAGE_END, strlen(MESSAGE_END));
        padding_size = MAX_MESSAGE_LEN - strlen(message);
        padding = (char *) calloc(padding_size, sizeof(char));
        file_size_written = write(filename_fd, padding, padding_size);
        close(filename_fd);
        
    } else if (strcmp(mode, READ_MODE) == 0) {
        printf("Reading message:\n");
        filename_fd = open(filename, O_RDONLY, S_IRWXU);
        file_size = MAX_MESSAGE_LEN + strlen(MESSAGE_END);
        offset = lseek(filename_fd, -file_size, SEEK_END);
        buf = (char *) malloc(file_size * sizeof(char));
        file_size_read = pread(filename_fd, buf, file_size, offset);
        close(filename_fd);

        for (message_end = MAX_MESSAGE_LEN; 
            (message_end >= 0) && (buf[message_end] != MESSAGE_END[0]); message_end--);
        for (i = 0; i < message_end; i++)
            printf("%c", buf[i]);
        printf("\n");        
    } else if (strcmp(mode, DELETE_MODE) == 0) {
        printf("Deleting message\n");
        filename_fd = open(filename, O_RDWR, S_IRWXU);
        offset = lseek(filename_fd, -(MAX_MESSAGE_LEN + strlen(MESSAGE_END)), SEEK_END);
        buf = (char *) malloc(offset * sizeof(char));
        file_size_read = pread(filename_fd, buf, offset, SEEK_SET);
        close(filename_fd);

        filename_fd = open(filename, O_WRONLY | O_TRUNC, S_IWUSR);
        file_size_written = pwrite(filename_fd, buf, file_size_read, SEEK_SET);
        close(filename_fd);
    } else {
        printf("Incorrect mode: %s\n", mode);
        return 1;
    }
    inspect_file(filename);
    return 0;
}
