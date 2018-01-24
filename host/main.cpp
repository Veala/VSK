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

void handle_error(char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
//    if (argc != 2) {
//        fprintf(stderr, "%s <dotted-address>\n", argv[0]);
//        exit(EXIT_FAILURE);
//    }
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (tcp_socket == -1)
        handle_error("tcp_socket error");

    sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = 3307;

    if (inet_aton("10.7.14.14", &my_addr.sin_addr) == 0)
        handle_error("inet_aton");

    if (bind(tcp_socket, (sockaddr *) &my_addr, sizeof(sockaddr_in)) == -1)
        handle_error("bind error");

    if (listen(tcp_socket, 1) == -1)
        handle_error("listen error");

    int selretval;
    timeval tv;



//	string str;
//	cout << "=>";
//	while (getline(cin, str)) {
//		if (str == "") {
			
//		} else if (str == "exit") {
//			break;
//		} else if (str == "start") {
//			break;
//		}
//		cout << "=> ";
//	}
	
    return 0;
}
