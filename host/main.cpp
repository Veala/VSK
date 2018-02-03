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
#define CMDSIZE 10
#define ERROR_TIME 1
#define ERROR_CONNECTION 2
#define ERROR_READ 3
#define NO_ERROR 4
#define checkError(err)\
        if (err == ERROR_CONNECTION) break;\
        if (err == ERROR_TIME) continue;\
        if (err == ERROR_READ) continue;\

using namespace std;

char message[SIZE];
char buf[SIZE];
char answer[CMDSIZE] = "answer";
char Ok[CMDSIZE] = "Ok";
char preSend[CMDSIZE];

int tcp_socket, rw_socket;
fd_set rfds;
timeval tv;
streampos fsize;
int intfsize;

void handle_error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void checkSend(ssize_t n) {
    if (n == -1)
        perror("write");
    printf("sent: %ld bytes\n", n);
}

int readData(int &n, ssize_t &r, int& size) {
    FD_ZERO(&rfds);
    FD_SET(rw_socket, &rfds);
    int retval = select(rw_socket+1, &rfds, NULL, NULL, &tv);
    if (retval) {
        if (FD_ISSET(rw_socket,&rfds)) {
            r = read(rw_socket, &buf[n], size - n);
            if (r == -1) {
                perror("read");
                return ERROR_READ;
            }
            if (r == 0) {
                perror("connection");
                return ERROR_CONNECTION;
            }
            printf("recv: %ld bytes\n", r);
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

int readAllData(int &n, int& size) {
    ssize_t r=0;
    while (1) {
        int err = readData(n, r, size);
        if (err != NO_ERROR) return err;
        n+=r;
        if (n>=size) return NO_ERROR;
    }
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        fprintf(stderr, "%s <dotted-address> <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    ifstream file(argv[2], ios_base::binary | ios_base::ate);
    if (file.is_open()) {
        fsize = file.tellg();
        intfsize = fsize;
        if (fsize > SIZE) {
            cout << "File size more than 500K" << endl;
            return 1;
        }
        file.seekg (0, ios_base::beg);
        file.read(message, fsize);
        file.close();
    } else {
        cout << "Unable to open file" << endl;
        return 1;
    }

    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

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

    sockaddr_in peer_addr;
    socklen_t peer_addr_size = sizeof(sockaddr_in);

    FD_ZERO(&rfds);
    FD_SET(tcp_socket, &rfds);
    tv.tv_sec = 5;
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
            sprintf(preSend, "%ld", (size_t)fsize);
            checkSend(write(rw_socket, &preSend, sizeof(preSend)));
            int recvCmdSize = sizeof(Ok), n=0;
            int err = readAllData(n, recvCmdSize);
            checkError(err)
            if ((n == recvCmdSize) && (strncmp(buf, Ok, recvCmdSize) == 0))
                    checkSend(write(rw_socket, &message, (size_t)fsize));
            else
                printf("\"Ok\" is not received\n");
        } else if (str == "recv") {
            checkSend(write(rw_socket, &answer, sizeof(answer)));
            int n=0;
            int err = readAllData(n, intfsize);
            checkError(err)
            if (n == intfsize)
                printf("Data received\n");
            else
                printf("Too big data\n");
        } else if (str == "cmp") {
            if (memcmp(buf, message, intfsize) == 0)
                printf("Ok, buf == message\n");
            else
                printf("Failed, buf != message\n");
        } else if (str == "ch") {
            getline(cin, str);
            strcpy(message, str.c_str());
            message[str.size()]='\n';
            message[str.size()+1]='\0';
            fsize = str.size()+2;
            intfsize = fsize;
        } else if (str == "save") {
            ofstream file("file2.pdf", ios_base::binary);
            if (file.is_open()) {
                file.write(buf, fsize);
                file.close();
            } else {
                cout << "Unable to open file" << endl;
                return 1;
            }
        }

        cout << "=>";
    }
	
    return 0;
}
