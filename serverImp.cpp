#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <iostream>

#include "server.hpp"


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
	std::cout << "exit" << std::endl;
}

void getMemorySize(char *memSize)
{
	struct rusage r_usage;
	getrusage(RUSAGE_SELF,&r_usage);
	sprintf(memSize, "%d", r_usage.ru_maxrss);
}

void getProcessList(char *data)
{
    FILE *pf;
    char cmd[20];

    sprintf(cmd, "ps aux");

    pf = popen(cmd,"r");

    fgets(data, 512 , pf);

    pclose(pf);

    write(1, data, 512);

    return;
}


int handleCommand(char *cmd, struct session *sess)
{
	int rc = 0;
	char memSize[50] = {0};
	char cwd[100] = {0};
	char processList[512] = {0};
	char unknown[] = "unknown";

	if(strstr(cmd, "stop") != 0) {
		rc = 1;
	} else if (strstr(cmd, "exit") != 0) {
		sess->s->serverRunning = 0;
		alarm(1);
		rc = 1;
	} else if (strstr(cmd, "status") != 0) {
		getMemorySize(memSize);
		if(send(sess->sock, memSize, strlen(memSize), 0) == -1)
			std::cout << "failed to send mem size" << std::endl;
		rc = 0;
	} else if (strstr(cmd, "path") != 0) {
		getcwd(cwd, sizeof(cwd));
		if(send(sess->sock, cwd, strlen(cwd) * sizeof(char), 0) == -1)
			std::cout << "failed to send working directory" << std::endl;
		rc = 0;
	} else if (strstr(cmd, "enumerate") != 0) {
		getProcessList(processList);
		if(send(sess->sock, processList, strlen(processList), 0) == -1)
			std::cout << "failed to send processList" << std::endl;
		rc = 0;
	} else {
		if(send(sess->sock, unknown, strlen(unknown), 0) == -1)
			std::cout << "failed to send unknown" << std::endl;
	}

	std::cout << sess->sock << ": " << cmd << std::endl;

	return rc;
}

void *sessionHandler(void *sessParam)
{
	struct session *sess = (struct session*)sessParam;
	int sessionSock = sess->sock;
	int dataSize = 0;
	char rcvBuf[100];

	std::cout << "Session " << sessionSock << " connected" << std::endl;

	do {
		memset(rcvBuf, 0, 100*sizeof(char));
		dataSize = recv(sessionSock, rcvBuf, 100, 0);

		if(dataSize <= 0)
			continue;

		if (handleCommand(rcvBuf, sess))
			break;

	} while ((dataSize > 0) && sess->s->serverRunning);

	close(sessionSock);
	std::cout << "Session " << sessionSock << " closed" << std::endl;
	pthread_exit(NULL);
}

int server::run()
{
	struct sockaddr_un local, remote;
	int sessionCount = 0, sessionSock = 0, listenSock = 0, session, len = 0;
	struct sigaction sa;
	struct session sess = {};

	if (maxSessions > MAX_SESSIONS) {
		std::cout << "max sessions over " << MAX_SESSIONS << std::endl;
		return -1;
	}

	sa.sa_handler = handleSigAlarm;
	sigaction(SIGALRM , &sa, NULL);
	listenSock = socket(AF_UNIX, SOCK_STREAM, 0);
	if( listenSock == -1) {
		std::cout << "Failed creating socket" << std::endl;
		return -1;
	}

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, sockPath);
	unlink(local.sun_path);
	len = strlen(local.sun_path) + sizeof(local.sun_family);

	if(bind(listenSock, (struct sockaddr*)&local, len) != 0) {
		std::cout << "Failed bind" << std::endl;
		return -1;
	}

	if (listen(listenSock, maxSessions) != 0)
		std::cout << "Failed listen" << std::endl;

	serverRunning = 1;
	while ((sessionCount < maxSessions) && serverRunning) {
		unsigned int sockLen = 0;

		std::cout << "Waiting..." << std::endl;
		if((sessionSock = accept(listenSock, (struct sockaddr*)&remote, &sockLen)) == -1) {
			if (serverRunning)
				std::cout << "Failed accept" << std::endl;
			break;
		}

		sess.s = this;
		sess.sock = sessionSock;
		if (pthread_create(&sessions[sessionCount++], NULL, sessionHandler, &sess)) {
			std::cout << "Failed to create session handler" << std::endl;
			break;
		}

		std::cout << "Created " << sessionCount << "/" <<  maxSessions << "sessions" << std::endl;
	}

	if (!serverRunning)
		for (session = 0; session < sessionCount; session++)
			pthread_cancel(sessions[session]);

	for (session = 0; session < sessionCount; session++)
		pthread_join(sessions[session], NULL);

	std::cout << "server exit" << std::endl;
	return 0;
}


