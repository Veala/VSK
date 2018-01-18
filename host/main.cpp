#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>

using namespace std;

int main(int argc, char* argv[])
{
	cout << "Hello world" << endl;
	int sfd = socket(AF_INET, SOCK_RAW, tcp);
	cout << "s: " << s << endl;
	

	string str;
	cout << "=>";
	while (getline(cin, str)) {
		if (str == "") {
			
		} else if (str == "exit") {
			break;
		} else if () {
			
		}
		cout << "=> ";
	}
	
	return 1;
}
