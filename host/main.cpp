//#include <iostream>
//#include <stdlib.h>
//#include <stdio.h>
//#include <sys/types.h>
//#include <sys/select.h>
//#include <sys/time.h>
//#include <unistd.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>

//using namespace std;

//void handle_error(char* msg) {
//    perror(msg);
//    exit(EXIT_FAILURE);
//}

//int main(int argc, char* argv[])
//{
////    if (argc != 2) {
////        fprintf(stderr, "%s <dotted-address>\n", argv[0]);
////        exit(EXIT_FAILURE);
////    }
//    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

//    if (tcp_socket == -1)
//        handle_error("tcp_socket error");

//    sockaddr_in my_addr;
//    my_addr.sin_family = AF_INET;
//    my_addr.sin_port = 3307;

//    if (inet_aton("10.7.14.14", &my_addr.sin_addr) == 0)
//        handle_error("inet_aton");

//    if (bind(tcp_socket, (sockaddr *) &my_addr, sizeof(sockaddr_in)) == -1)
//        handle_error("bind error");

//    if (listen(tcp_socket, 1) == -1)
//        handle_error("listen error");

//    int selretval;
//    timeval tv;



////	string str;
////	cout << "=>";
////	while (getline(cin, str)) {
////		if (str == "") {
			
////		} else if (str == "exit") {
////			break;
////		} else if (str == "start") {
////			break;
////		}
////		cout << "=> ";
////	}
	
//    return 0;
//}

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

using namespace std;
int
main(void) {
    fd_set rfds;
    struct timeval tv;
    int retval;

    /* Следим ввели ли что-либо в stdin (fd 0). */
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    /* Ждем до 5 секунд. */
    tv.tv_sec = 5;
    tv.tv_usec = 0;
//    while (1) {
        retval = select(1, &rfds, NULL, NULL, &tv);
        /* На значение tv в данный момент полагаться нельзя! */
        cout << "retval: " << retval << endl;

        if (retval == -1) {
            cout << "gopa";
            perror("error");
        }
        else if (retval) {
            printf("Данные доступны.\n");
            //break;


        /* FD_ISSET(0, &rfds) will be true. */
        }
        else
            printf("Нет данных в течение 5 секунд.\n");
//    }

    exit(0);
}
