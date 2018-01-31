#include <iostream>
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

using namespace std;

char message[1024];
char buf[1024];
const char* answer = "answer me\n";

void handle_error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
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
    while (1) {
        FD_ZERO(&rfds);
        FD_SET(tcp_socket, &rfds);
        int retval = select(tcp_socket+1, &rfds, NULL, NULL, NULL);
        if (retval) {
            if (FD_ISSET(tcp_socket,&rfds)) {
                ssize_t r = read(tcp_socket, &buf, sizeof(buf));
                if (r == -1)
                    perror("read");
                if (r == 0)
                    break;
                if (strcmp(buf, answer) == 0) {
                    if (write(tcp_socket, &message, sizeof(message)) == -1)
                        perror("write");
                } else {
                    printf(buf);
                    strcpy(message, buf);
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
