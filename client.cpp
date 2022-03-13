#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <iostream>
using namespace std;

static const char* socket_path = "/home/daniel/yonienv/share/code/cynet/cynet-socket";
static const unsigned int s_send_len = 100;

int main()
{
	struct sockaddr_un remote;
	char send_msg[s_send_len];
	int data_len = 0, sock = 0;

	memset(send_msg, 0, s_send_len*sizeof(char));

	if( (sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		cout << "Failed creating socket\n";
		return 1;
	}

	remote.sun_family = AF_UNIX;
	strcpy( remote.sun_path, socket_path );
	data_len = strlen(remote.sun_path) + sizeof(remote.sun_family);

	if (connect(sock, (struct sockaddr*)&remote, data_len) == -1) {
		cout << "Failed to connect\n";
		return 1;
	}

	cout << "Connected\n";

	while(1) {
		memset(send_msg, 0, s_send_len * sizeof(char));
		cout << "> ";
		cin >> send_msg;
		if( send(sock, send_msg, strlen(send_msg)*sizeof(char), 0 ) == -1 ) {
			cout << "failed send\n";
		}

		if (strstr(send_msg, "stop") != 0 || strstr(send_msg, "exit") != 0) {
			break;
		}
	}

	close(sock);
	cout << "Session " << sock << "closed\n";

	return 0;
}
