#include <stdio.h> // printf
#include <sys/stat.h> // mkdir
#include <string.h> // strcat
#include <stdlib.h> // malloc

// . hide. 
// Напишите программу, скрывающую файл, заданный в качестве аргумента, в "темном" каталоге. 
// Имя файла передайте с помощью аргументов командной строки. 
// Имя каталога - фиксированное. Максмальный балл - 5.

#define DARK_DIR "dark"


void create_if_not_exists(char *dir) {
    int result = mkdir(dir, 0333);
}


int main(int argc, char **argv) {
    char *filename = argv[1];
    char *new_filename;
    printf("Processing file: %s\n", filename);
    create_if_not_exists(DARK_DIR);
    new_filename = (char *) malloc ((strlen(DARK_DIR) + strlen(filename) + 1) * sizeof (char));
    strcpy(new_filename, DARK_DIR);
    strcat(new_filename, "/");
    strcat(new_filename, filename);
    rename(filename, new_filename);
    return 0;
}
