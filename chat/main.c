#include <stdio.h> // printf
#include <unistd.h> // getcwd
#include <stdlib.h> // free
#include <dirent.h> // opendir
#include <sys/stat.h> // stat
#include <string.h> // strcmp
#include <sys/types.h> // mode_t
#include <sys/socket.h> // socket
#include <arpa/inet.h>// sockaddr_in,  htons
#include <signal.h> // SIGKILL
#include <assert.h> // assert


// 2. Написать простой консольный клиент чата, 
// позволяющий обмениваться текстовыми сообщениями с такими же клиентами в локальной сети. 
// Для передачи сообщений используйте широковещательные UDP дейтаграммы. 
// Порт укажите в аргументе командной строки.
// Максимальный балл — 15.


#define MIN_PORT_NUMBER 10000
#define MAX_PORT_NUMBER 10100
#define MAX_MESSAGE_LEN 100
#define MESSAGE_PREFIX "Client from port "
#define STRINGIFY(x) #x
#define MESSAGE_PREFIX_LEN strlen(MESSAGE_PREFIX) + strlen(STRINGIFY(MAX_PORT_NUMBER))
#define ENTER_MESSAGE "> "


int get_socket() {
    int sockfd;
    int broadcast = 1;
    if ((sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket");
        exit(1);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) == -1) {
        perror("setsockopt(SO_BROADCAST)");
        exit(1);
    }
    return sockfd;
}

static volatile int is_running = 1;


void sigint_handler(int sig) {
    is_running = 0;
}
   

int main(int argc, char** argv) {
    int port;
    int iter_port;
    int sockfd;
    struct sockaddr_in recv_addr;  
    struct sockaddr_in sender_addr;
    int sender_addr_len;
    pid_t pid;
    char *message = NULL;
    char *read_message = NULL;
    char recvbuff[MAX_MESSAGE_LEN];
    if (!(argv[1])) {
        perror("port argument");
        exit(1);
    }
    signal(SIGINT, sigint_handler);

    port = atoi(argv[1]);
    assert((port >= MIN_PORT_NUMBER) && (port <= MAX_PORT_NUMBER));

    recv_addr.sin_family = AF_INET;        
    recv_addr.sin_port = htons(port);   
    recv_addr.sin_addr.s_addr = INADDR_ANY;

    pid = fork();
    if (pid == 0) { // in child process
        sockfd = get_socket();
        printf("Launched child process %d\n", getpid());
        fflush(stdout);

        if (bind(sockfd, (struct sockaddr*)&recv_addr, sizeof(recv_addr)) < 0) {
            perror("bind");
            exit(1);
        }
        while (1) {
            // printf("what1"); fflush(stdout);
            if (recvfrom(sockfd, recvbuff, MAX_MESSAGE_LEN, 0, (struct sockaddr *)&sender_addr, &sender_addr_len) == -1 ) {
                perror("recvfrom");
                exit(1);
            }
            // printf("what2"); fflush(stdout);
            printf("\n\t\t\t%s", recvbuff);
            printf(ENTER_MESSAGE);
            fflush(stdout);
        }
        exit(0);
    } else if (pid < 0) {
        perror("fork");
        exit(1);
    }

    sockfd = get_socket();
    sleep(2);

    while (is_running) {
        printf(ENTER_MESSAGE);
        fflush(stdout);
        if (message != NULL)
            free(message);
        message = (char *) malloc((MAX_MESSAGE_LEN + MESSAGE_PREFIX_LEN + 1) * sizeof(char));
        strcpy(message, MESSAGE_PREFIX);
        strcat(message, argv[1]);
        strcat(message, ": ");
        if (read_message != NULL)
            free(read_message);
        read_message = (char *) malloc((MAX_MESSAGE_LEN + 1) * sizeof(char));
        fgets(read_message, MAX_MESSAGE_LEN, stdin);
        strcat(message, read_message);

        for (iter_port = MIN_PORT_NUMBER; iter_port <= MAX_PORT_NUMBER; iter_port++) {
            if (iter_port == port) 
                continue;
            recv_addr.sin_port = htons(iter_port);  
            if (sendto(sockfd, message, strlen(message) + 1, 0, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) == -1) {
                perror("sendto");
                exit(1);
            }
        }        
    }

    if (kill(pid, SIGKILL) < 0) {
        perror("kill");
        exit(1);
    }

    close(sockfd);

    printf("Closed\n");
    fflush(stdout);
}

