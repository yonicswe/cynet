#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
using namespace std;

#define MSG_SIZE 100

int main(int argc, char *argv[])
{
	struct sockaddr_un remote;
	char sendMsg[MSG_SIZE];
	struct stat statBuffer;
	int dataLen = 0, sock = 0;
	char *sockPath;

	if (argc < 2)
		cout << "usage: ./client <socket path>\n";

	sockPath = argv[1];
	if (stat(sockPath, &statBuffer)) {
		cout << "file " << sockPath << " does not exist\n";
		return -1;
	}

	memset(sendMsg, 0, MSG_SIZE * sizeof(char));

	if( (sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		cout << "Failed creating socket\n";
		return 1;
	}

	remote.sun_family = AF_UNIX;
	strcpy( remote.sun_path, sockPath );
	dataLen = strlen(remote.sun_path) + sizeof(remote.sun_family);

	if (connect(sock, (struct sockaddr*)&remote, dataLen) == -1) {
		cout << "Failed to connect\n";
		return 1;
	}

	cout << "Connected\n";

	while(1) {
		memset(sendMsg, 0, MSG_SIZE * sizeof(char));
		cout << "> ";
		cin >> sendMsg;
		if(send(sock, sendMsg, strlen(sendMsg) * sizeof(char), 0 ) == -1) {
			cout << "failed send\n";
		}

		if (strstr(sendMsg, "stop") != 0 || strstr(sendMsg, "exit") != 0) {
			break;
		}
	}

	close(sock);
	cout << "Session " << sock << "closed\n";

	return 0;
}
