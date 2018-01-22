#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

int main(int argc, char* argv[])
{
//    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
//    if (tcp_socket == -1) {
//        perror("sfd");
//        exit(2);
//    }

    sockaddr_in my_addr;
    if (inet_aton("127.0.0.1", &my_addr.sin_addr) == 0) {
        perror("inet_anot");
        exit(EXIT_FAILURE);
    }
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = 3307;

    printf("%s\n", inet_ntoa(my_addr.sin_addr));
	
    //int bnd = bind(sfd, &my_addr, sizeof(sockaddr_in));

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
	
	return 1;
}
