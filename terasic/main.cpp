#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define SIZE 500000

using namespace std;

char message[SIZE];
char buf[SIZE];
const char* answer = "answer me\n";
const char* preSend = "send";
unsigned int size = 0;

char * strncpy_my(char *dest, const char *src, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++)
        dest[i] = src[i];
    return dest;
}

void handle_error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void checkSend(ssize_t n) {
    if (n == -1)
        perror("write");
    printf("sent: %ld bytes\n", n);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "%s <dotted-address-host> <dotted-address-server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
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

    fd_set rfds;
    unsigned int curSize = 0;
    ssize_t r=0;
    enum {
        fBuf,
        fMes
    } flag;
    flag = fBuf;
    while (1) {
        FD_ZERO(&rfds);
        FD_SET(tcp_socket, &rfds);
        int retval = select(tcp_socket+1, &rfds, NULL, NULL, NULL);
        if (retval) {
            if (FD_ISSET(tcp_socket,&rfds)) {
                if (flag == fBuf) r = read(tcp_socket, buf, sizeof(buf));
                if (flag == fMes) r = read(tcp_socket, &message[curSize], size-curSize);

                if (r == -1) perror("read");
                if (r == 0) break;
                printf("recv: %ld bytes\n", r);

                if(flag == fMes) {
                    curSize+=r;
                    if (curSize >= size) flag = fBuf;
                } else if(flag == fBuf) {
                    if (strncmp(buf, answer, sizeof(answer)) == 0) {
                        checkSend(write(tcp_socket, &message, size));
                    } else if (r>0 && r<=10) {
                        size=atoi(buf);
                        printf("size: %d\n", size);
                        curSize=0;
                        flag = fMes;
                        checkSend(write(tcp_socket, "Ok", 2));
                    } else {
                        printf("recv unknown cmd\n");
                    }
                }
            }
        } else if(retval == -1) {
            handle_error("select -1");
        } else {
            handle_error("select");
        }
    }

    if (close(tcp_socket) == -1)
        handle_error("close tcp socket");

    return 0;
}
