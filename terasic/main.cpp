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

using namespace std;

void handle_error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket == -1)
        handle_error("tcp_socket error");

    sockaddr_in my_addr, peer_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = 3308;
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = 3307;

    if (inet_aton("10.7.14.14", &my_addr.sin_addr) == 0)
        handle_error("inet_aton");

    if (inet_aton("10.7.14.14", &peer_addr.sin_addr) == 0)
        handle_error("inet_aton");

    if (bind(tcp_socket, (sockaddr *) &my_addr, sizeof(sockaddr_in)) == -1)
        handle_error("bind");

    if (connect(tcp_socket, (sockaddr *) &peer_addr, sizeof(sockaddr_in)) == -1)
        handle_error("connect");

    printf("connected!\n");

    fd_set rfds;
    fd_set wfds;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_SET(tcp_socket, &rfds);
    FD_SET(tcp_socket, &wfds);
    while (1) {
        int retval = select(tcp_socket+1, &rfds, &wfds, NULL, NULL);
        if (retval) {
            if (FD_ISSET(tcp_socket,&rfds)) {

            } else if (FD_ISSET(tcp_socket,&wfds)) {

            }
        } else if(retval == -1) {
            handle_error("select");
        }
    }

    string str;
    cout << "=>";
    while (getline(cin, str)) {
        if (str == "") {

        } else if (str == "exit") {
            if (close(tcp_socket) == -1)
                handle_error("close tcp socket");
            break;
        } else if (str == "start") {
            break;
        }
        cout << "=>";
    }
    return 0;
}
