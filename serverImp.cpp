#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <iostream>

#include "server.hpp"

using namespace std;

struct session {
	server *s;
	int sock;
};

server::server(char *_sockPath, int _maxSessions):sockPath(_sockPath),maxSessions(_maxSessions)
{
	serverRunning = 0;
};

void handleSigAlarm(int sig)
{
	cout << "exit\n";
}

int handleCommand(char *cmd, struct session *sess)
{
	int rc = 0;

	if(strstr(cmd, "stop") != 0) {
		rc = 1;
	} else if (strstr(cmd, "exit") != 0) {
		sess->s->serverRunning = 0;
		alarm(1);
		rc = 1;
	} else if (strstr(cmd, "status") != 0) {
		rc = 0;
	} else if (strstr(cmd, "path") != 0) {
		rc = 0;
	} else if (strstr(cmd, "enumerate") != 0) {
		rc = 0;
	} else {
		cout << "unknown command\n";
	}

	cout << sess->sock << ": " << cmd << "\n";

	return rc;
}

void *sessionHandler(void *sessParam)
{
	struct session *sess = (struct session*)sessParam;
	int sessionSock = sess->sock;
	int dataSize = 0;
	char rcvBuf[100];

	cout << "Session " << sessionSock << " connected\n";

	do {
		memset(rcvBuf, 0, 100*sizeof(char));
		dataSize = recv(sessionSock, rcvBuf, 100, 0);

		if(dataSize <= 0)
			continue;

		if (handleCommand(rcvBuf, sess))
			break;

	} while ((dataSize > 0) && sess->s->serverRunning);

	close(sessionSock);
	cout << "Session " << sessionSock << " closed\n";
	pthread_exit(NULL);
}

int server::run()
{
	struct sockaddr_un local, remote;
	int sessionCount = 0, sessionSock = 0, listenSock = 0, session, len = 0;
	struct sigaction sa;
	struct session sess = {};

	if (maxSessions > MAX_SESSIONS) {
		cout << "max sessions over " << MAX_SESSIONS << "\n";
		return -1;
	}

	sa.sa_handler = handleSigAlarm;
	sigaction(SIGALRM , &sa, NULL);
	listenSock = socket(AF_UNIX, SOCK_STREAM, 0);
	if( listenSock == -1) {
		cout << "Failed creating socket\n";
		return -1;
	}

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, sockPath);
	unlink(local.sun_path);
	len = strlen(local.sun_path) + sizeof(local.sun_family);

	if(bind(listenSock, (struct sockaddr*)&local, len) != 0) {
		cout << "Failed bind\n";
		return -1;
	}

	if (listen(listenSock, maxSessions) != 0)
		cout << "Failed listen\n";

	serverRunning = 1;
	while ((sessionCount < maxSessions) && serverRunning) {
		unsigned int sockLen = 0;

		cout << "Waiting...\n";
		if((sessionSock = accept(listenSock, (struct sockaddr*)&remote, &sockLen)) == -1) {
			if (serverRunning)
				cout << "Failed accept\n";
			break;
		}

		sess.s = this;
		sess.sock = sessionSock;
		if (pthread_create(&sessions[sessionCount++], NULL, sessionHandler, &sess)) {
			cout << "Failed to create session handler\n";
			break;
		}

		cout << "Created " << sessionCount << "/" <<  maxSessions << "sessions\n";
	}

	if (!serverRunning)
		for (session = 0; session < sessionCount; session++)
			pthread_cancel(sessions[session]);

	for (session = 0; session < sessionCount; session++)
		pthread_join(sessions[session], NULL);

	cout << "server exit\n";
	return 0;
}


