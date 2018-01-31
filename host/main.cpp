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

char message[] = "Hello Terasic!\n";
char buf[1024];
char answer[] = "answer me\n";

void handle_error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "%s <dotted-address>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (tcp_socket == -1)
        handle_error("tcp_socket error");

    sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(3307);

    if (inet_aton(argv[1], &my_addr.sin_addr) == 0)
        handle_error("inet_aton");

    if (bind(tcp_socket, (sockaddr *) &my_addr, sizeof(sockaddr_in)) == -1)
        handle_error("bind");

    if (listen(tcp_socket, 1) == -1)
        handle_error("listen");

    printf("listening...\n");

    fd_set rfds;
    timeval tv;
    int rw_socket;
    sockaddr_in peer_addr;
    socklen_t peer_addr_size = sizeof(sockaddr_in);

    FD_ZERO(&rfds);
    FD_SET(tcp_socket, &rfds);
    tv.tv_sec = 50;
    tv.tv_usec = 0;
    int retval = select(tcp_socket+1, &rfds, NULL, NULL, &tv);
    if (retval) {
        if (FD_ISSET(tcp_socket,&rfds)) {
            printf("Connection is available now.\n");
            rw_socket = accept(tcp_socket, (sockaddr*) &peer_addr, &peer_addr_size);
            if (rw_socket == -1)
                handle_error("accept");
        } else {
            FD_ZERO(&rfds);
            printf("No connection.\n");
            return 1;
        }
    } else {
        FD_ZERO(&rfds);
        printf("No connection within five seconds.\n");
        return 1;
    }

    if (close(tcp_socket) == -1)
        handle_error("close tcp socket");

    string str;
    cout << "=>";
    while (getline(cin, str)) {
        if (str == "") {
			
        } else if (str == "exit") {
            if (close(rw_socket) == -1)
                handle_error("close rw socket");
            break;
        } else if (str == "send") {
            if (write(rw_socket, &message, sizeof(message)) == -1)
                perror("write");
        } else if (str == "recieve") {
            if (write(rw_socket, &answer, sizeof(answer)) == -1)
                perror("write");
            while (1) {
                FD_ZERO(&rfds);
                FD_SET(rw_socket, &rfds);
                retval = select(rw_socket+1, &rfds, NULL, NULL, NULL);
                if (retval) {
                    if (FD_ISSET(rw_socket,&rfds)) {
                        if (read(rw_socket, &buf, sizeof(buf)) == -1)
                            perror("read");
                        printf(buf);
                        break;
                    }
                } else if(retval == -1) {
                    cout << retval << endl;
                    handle_error("select");
                }
            }
        } else if (str == "compare") {
            if (strcmp(buf, message) == 0)
                printf("Ok, buf == message\n");
            else
                printf("Failed, buf != message\n");
        } else if (str == "ch") {
            getline(cin, str);
            strcpy(message, str.c_str());
            message[str.size()]='\n';
            message[str.size()+1]='\0';
        }

        cout << "=>";
    }
	
    return 0;
}
