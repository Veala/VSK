#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>

using namespace std;

int main(int argc, char* argv[])
{
	cout << "Hello world" << endl;
	int sfd = socket(AF_INET, SOCK_STREAM, TCP);
//	if (sfd == -1) {
//		perror("sfd");
//		exit(2);
//	}

//	sockaddr_in my_addr;
	
//	int bnd = bind(sfd, &my_addr, sizeof(sockaddr_in));
//	sock

	string str;
	cout << "=>";
	while (getline(cin, str)) {
		if (str == "") {
			
		} else if (str == "exit") {
			break;
		} else if (str == "start") {
			break;
		}
		cout << "=> ";
	}
	
	return 1;
}
