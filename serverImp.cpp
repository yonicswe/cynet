#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

#include "server.hpp"


struct session {
	server *s;
	int sock;
};

server::server(std::string _sockPath, int _maxSessions):sockPath(_sockPath),maxSessions(_maxSessions)
{
	serverRunning = 0;
};

void server::handleSigAlarm(int sig)
{
	std::cout << "exit" << std::endl;
}

int server::handleCommand(char *cmd, struct session *sess)
{
	int rc = 0;

	std::cout << sess->sock << ": " << cmd << std::endl;

	auto it = commandTable.find(std::string(cmd));
	if (it != commandTable.end()) {
		switch (it->second) {
			case 1: // stop
				rc = 1;
				break;
			case 2: // exit
				sess->s->serverRunning = 0;
				alarm(1);
				rc = 1;
				break;
			case 3: // status
				{
					std::string memSize(sysUtils::getMemorySize());
					if(send(sess->sock, memSize.c_str(), memSize.size(), 0) == -1)
						std::cout << "failed to send mem size" << std::endl;
					rc = 0;
				}
				break;
			case 4: // path
				{
					char cwd[100] = {0};
					getcwd(cwd, sizeof(cwd));
					if(send(sess->sock, cwd, strlen(cwd) * sizeof(char), 0) == -1)
						std::cout << "failed to send working directory" << std::endl;
					rc = 0;
				}
				break;
			case 5: // enumerate
				{
					std::string processList(sysUtils::getProcessList());
					std::cout << processList;
					if(send(sess->sock, processList.c_str(), processList.size(), 0) == -1)
						std::cout << "failed to send processList" << std::endl;
					rc = 0;
				}
				break;
			default: // unknown
				std::cout << "Unknown" << std::endl;
				break;
		}
	} else {
		char unknown[] = "unknown";
		if(send(sess->sock, unknown, strlen(unknown), 0) == -1)
			std::cout << "failed to send unknown" << std::endl;
	}

	return rc;
}

void server::sessionHandler(void *sessParam)
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
}

std::map<std::string, int> server::commandTable;
int server::run()
{
	struct sockaddr_un local, remote;
	int sessionCount = 0, sessionSock = 0, listenSock = 0, session, len = 0;
	struct sigaction sa;
	struct session sess = {};

	if (maxSessions > server::kMaxSessions) {
		std::cout << "max sessions over " << server::kMaxSessions << std::endl;
		return -1;
	}

	commandTable[std::string("stop")] = 1;
	commandTable[std::string("exit")] = 2;
	commandTable[std::string("status")] = 3;
	commandTable[std::string("path")] = 4;
	commandTable[std::string("enumerate")] = 5;

	sa.sa_handler = handleSigAlarm;
	sigaction(SIGALRM , &sa, NULL);
	listenSock = socket(AF_UNIX, SOCK_STREAM, 0);
	if( listenSock == -1) {
		std::cout << "Failed creating socket" << std::endl;
		return -1;
	}

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, sockPath.c_str());
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
		sessions.push_back(std::thread(sessionHandler,  &sess));
		sessionCount++;

		std::cout << "Created " << sessionCount << "/" <<  maxSessions << "sessions" << std::endl;
	}

	if (!serverRunning)
		for (auto it = sessions.begin(); it != sessions.end(); it++)
			pthread_cancel(it->native_handle());

	for (auto it = sessions.begin(); it != sessions.end(); it++)
		it->join();

	std::cout << "server exit" << std::endl;
	return 0;
}


