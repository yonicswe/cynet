#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>

#define MSG_SIZE 100

int main(int argc, char *argv[])
{
	struct sockaddr_un remote;
	char sendMsg[MSG_SIZE];
	char rcvMsg[MSG_SIZE];
	struct stat statBuffer;
	int dataLen = 0, sock = 0;
	char *sockPath;

	if (argc < 2)
		std::cout << "usage: ./client <socket path>" << std::endl;

	sockPath = argv[1];
	if (stat(sockPath, &statBuffer)) {
		std::cout << "file " << sockPath << " does not exist" << std::endl;
		return -1;
	}

	memset(sendMsg, 0, MSG_SIZE * sizeof(char));

	if( (sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		std::cout << "Failed creating socket" << std::endl;
		return 1;
	}

	remote.sun_family = AF_UNIX;
	strcpy( remote.sun_path, sockPath );
	dataLen = strlen(remote.sun_path) + sizeof(remote.sun_family);

	if (connect(sock, (struct sockaddr*)&remote, dataLen) == -1) {
		std::cout << "Failed to connect" << std::endl;
		return 1;
	}

	std::cout << "Connected" << std::endl;

	while(1) {
		memset(sendMsg, 0, MSG_SIZE * sizeof(char));
		std::cout << "> ";
		std::cin >> sendMsg;
		if(send(sock, sendMsg, strlen(sendMsg) * sizeof(char), 0 ) == -1) {
			std::cout << "failed send" << std::endl;
		}

		if (strstr(sendMsg, "stop") != 0 || strstr(sendMsg, "exit") != 0) {
			break;
		}

		if( recv(sock, rcvMsg, MSG_SIZE, 0) > 0 )
			std::cout << rcvMsg << std::endl;
	}

	close(sock);
	std::cout << "Session " << sock << "closed" << std::endl;

	return 0;
}
