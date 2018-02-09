#include <stdlib.h>
#include <stdio.h>
//#include <sys/types.h>
//#include <sys/select.h>
//#include <sys/time.h>
#include <unistd.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

//#define debug

#define SIZE 2000000
#define CMDSIZE 10
#define ERROR_TIME 1
#define ERROR_CONNECTION 2
#define ERROR_READ 3
#define NO_ERROR 4
#define checkError(err)\
        if (err == ERROR_CONNECTION) break;\
        if (err == ERROR_TIME) { tv.tv_sec = 5; continue; }\
        if (err == ERROR_READ) continue;\

char message[SIZE];
char buf[SIZE];
char answer[CMDSIZE] = "answer";
//const char* preSend = "send";
char Ok[CMDSIZE] = "Ok";

int tcp_socket;
fd_set rfds;
timeval tv;
int size = 0;

void handle_error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void checkSend(ssize_t n) {
    if (n == -1)
        perror("write");
    #ifdef debug
    printf("sent: %ld bytes\n", n);
    #endif
}

int readData(int &n, ssize_t &r, int& size, char (&refArray)[SIZE], timeval* tv) {
    FD_ZERO(&rfds);
    FD_SET(tcp_socket, &rfds);
    int retval = select(tcp_socket+1, &rfds, NULL, NULL, tv);
    if (retval) {
        if (FD_ISSET(tcp_socket,&rfds)) {
            r = read(tcp_socket, &refArray[n], size - n);
            if (r == -1) {
                perror("read");
                return ERROR_READ;
            }
            if (r == 0) {
                perror("connection");
                return ERROR_CONNECTION;
            }
            #ifdef debug
            printf("recv: %ld bytes\n", r);
            #endif
            return NO_ERROR;
        }
    } else if(retval == -1) {
        handle_error("select");
    } else {
        FD_ZERO(&rfds);
        printf("No data within five seconds.\n");
        return ERROR_TIME;
    }
}

int readAllData(int &n, int& size, char (&refArray)[SIZE], timeval* tv) {
    ssize_t r=0;
    while (1) {
        int err = readData(n, r, size, refArray, tv);
        if (err != NO_ERROR) return err;
        n+=r;
        if (n>=size) return NO_ERROR;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "%s <dotted-address-host> <dotted-address-server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket == -1)
        handle_error("tcp_socket error");

    sockaddr_in my_addr, peer_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(3308);
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(3307);

    if (inet_aton(argv[1], &my_addr.sin_addr) == 0)
        handle_error("inet_aton");

    if (inet_aton(argv[2], &peer_addr.sin_addr) == 0)
        handle_error("inet_aton");

    if (bind(tcp_socket, (sockaddr *) &my_addr, sizeof(sockaddr_in)) == -1)
        handle_error("bind");

    if (connect(tcp_socket, (sockaddr *) &peer_addr, sizeof(sockaddr_in)) == -1)
        handle_error("connect");

    printf("connected!\n");

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    while (1) {
        int n=0;
        int recvCmdSize = sizeof(Ok); // or sizeof(answer) == CMDSIZE
        int err = readAllData(n, recvCmdSize, buf, NULL);
        checkError(err)
        if ((n == recvCmdSize) && (strcmp(buf, answer) == 0)) {
            checkSend(write(tcp_socket, &message, size));
        } else if (n == recvCmdSize) {
            size=atoi(buf);
            #ifdef debug
            printf("size: %d\n", size);
            #endif
            checkSend(write(tcp_socket, Ok, sizeof(Ok)));
            n=0;
            err = readAllData(n, size, message, &tv);
            checkError(err)
            checkSend(write(tcp_socket, Ok, sizeof(Ok)));
        }
    }

    if (close(tcp_socket) == -1)
        handle_error("close tcp socket");

    return 0;
}
