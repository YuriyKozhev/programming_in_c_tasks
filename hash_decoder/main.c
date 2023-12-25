#include <stdio.h> // printf
#include <unistd.h> // getcwd
#include <stdlib.h> // free
#include <dirent.h> // opendir
#include <sys/stat.h> // stat
#include <string.h> // strcmp
#include <sys/types.h> // mode_t
#include <openssl/md5.h> // md5
#include <sys/wait.h> // wait
#include <signal.h> // SIGKILL


// 1. Напишите программу, 
// выполняющую взлом хэша md5 с возможностью параллельного расчета в нескольких процессах. 
// Значение хэша передайте в качестве аргумента командной строки. 
// Количество процессов дожно выбираться автоматически. 
// Для расчета хеша можно исполользовать библиотеку openssl.
// Максимальный балл — 10.


#define MIN_ASCII_NUM 32
#define MAX_ASCII_NUM 126
#define BUFFER_LENGTH 100
#define MD5_DIGEST_LEN 16


void compute_md5(const char *str, char hashed_string[2 * MD5_DIGEST_LEN + 1]) {
    unsigned char digest[MD5_DIGEST_LEN];
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, str, strlen(str));
    MD5_Final(digest, &ctx);
    for (int i = 0, j = 0; i < MD5_DIGEST_LEN; i++, j+=2)
        sprintf(hashed_string + j, "%02x", digest[i]);
    hashed_string[2 * MD5_DIGEST_LEN] = 0;
}


int cmp_hash(const char *string, const char *hash) {
    int result;
    char hashed_string[2 * MD5_DIGEST_LEN + 1];
    compute_md5(string, hashed_string);
    result = strcmp(hashed_string, hash);
    return result;
}


int check(const char *hash, char *string, int length, int current_position) {
    int i = 0;
    int result;
    if (current_position == length) {
        if (cmp_hash(string, hash) == 0) {
            printf("%s", string);
            fflush(stdout);
            return 0;
        }
        return 1;
    }
    for (i = MIN_ASCII_NUM; i <= MAX_ASCII_NUM; i++) {
        string[current_position] = (char) i;
        result = check(hash, string, length, current_position + 1);
        if (result == 0)
            return 0;
    }
    return 1;
}


void decode_hash(const char *hash, int child_id, int process_number) {
    int length;
    int result;
    char *string;

    for (length = 1; 1; length++) {
        if (length % process_number == child_id) {
            string = (char *) malloc((length + 1) * sizeof(char));
            string[length] = '\0';
            result = check(hash, string, length, 0);
            free(string);
            if (result == 0) {
                return;
            }
        }
    }
}


void kill_children(pid_t *children, int process_number, pid_t pid_except) {
    int child_id;
    pid_t pid;
    for (child_id = 0; child_id < process_number; child_id++) {
        pid = children[child_id];
        if (pid == pid_except)
            continue;
        if (kill(pid, SIGKILL) < 0) {
            perror("kill");
            exit(1);
        }
    }
    printf("Killed children\n");
    fflush(stdout);
}


static volatile int is_running = 1;


void sigint_handler(int sig) {
    is_running = 0;
}
   
char *read_from_stdin() {
    char buf[BUFFER_LENGTH];
    int size_read = 0;
    int curr_length = 0;
    char *all_data = NULL;
    char *tmp_data;
    while (1) {
        size_read = read(0, buf, BUFFER_LENGTH);
        if ((size_read < 0) || ((all_data == NULL) && (size_read == 0))) {
            perror("read");
            exit(1);
        }
        if (size_read == 0)
            break;
        curr_length += size_read;
        tmp_data = (char *) malloc((curr_length + 1) * sizeof(char));
        if (all_data == NULL) {
            all_data = tmp_data;
            strcpy(all_data, buf);
        } else {
            strcpy(tmp_data, all_data);
            strcat(tmp_data, buf);
            free(all_data);
            all_data = tmp_data;
        }
        all_data[curr_length] = '\0';
        fflush(stdout);
    }
    return all_data;
}


int main(int argc, char** argv) {
    int child_id = 0;
    int cpu_number = sysconf(_SC_NPROCESSORS_ONLN);
    int process_number = cpu_number - 1;
    char *hash = argv[1];
    int child_status;
    pid_t pid;
    pid_t *children;
    int fd[2];
    char *buf = NULL;

    pipe(fd);

    if (hash == NULL) {
        exit(1);
    }

    signal(SIGINT, sigint_handler);

    children = (pid_t *) malloc(process_number * sizeof(pid_t));

    printf("Main pid = %d\n", getpid());
    printf("Processing hash = \"%s\"\n", hash);
    printf("Launching %d processes\n", process_number);

    for (child_id = 0; child_id < process_number; child_id++) {
        pid = fork();
        children[child_id] = pid;
        if (pid == 0) { // in child process
            printf("Launched child process %d\n", getpid());
            fflush(stdout);
            dup2(fd[1], 1); // standard output to pipe
            close(fd[0]); // standard input
            decode_hash(hash, child_id, process_number);
            exit(0);
        } else if (pid > 0) {
            // in parent process
        } else if (pid < 0) {
            perror("fork");
            exit(1);
        }
    }
    dup2(fd[0], 0); // standard input from pipe
    close(fd[1]); // standard output
    while (is_running) {
        for (child_id = 0; child_id < process_number; child_id++) {
            pid = children[child_id];
            if (waitpid(pid, &child_status, WNOHANG) != 0) {
                is_running = 0;
                break; 
            }
        }
        sleep(1);
    }  
    kill_children(children, process_number, pid);
    if (!WIFEXITED(child_status)) {
        perror("child_status");
        exit(1);
    }
    buf = read_from_stdin();
    if (buf == NULL) {
        perror("read_from_stdin");
        exit(1);
    }
    printf("Child process %d has completed with found string = '%s'\n", pid, buf);
    return 0;
}
