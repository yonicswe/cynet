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
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

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

void server::getMemorySize(std::string &memSize)
{
	struct rusage r_usage;
	getrusage(RUSAGE_SELF,&r_usage);
	std::ostringstream m;
	m << r_usage.ru_maxrss;
	memSize = m.str();
}

void server::getProcessList(std::string &data)
{
	std::system("ps aux > proc_list.txt" );
	//std::cout << std::ifstream("proc_list.txt").rdbuf();
	std::getline(std::ifstream("proc_list.txt"), data, '\0');
	return;
}

int server::handleCommand(char *cmd, struct session *sess)
{
	int rc = 0;

	if(strstr(cmd, "stop") != 0) {
		rc = 1;
	} else if (strstr(cmd, "exit") != 0) {
		sess->s->serverRunning = 0;
		alarm(1);
		rc = 1;
	} else if (strstr(cmd, "status") != 0) {
		std::string memSize;
		getMemorySize(memSize);
		if(send(sess->sock, memSize.c_str(), memSize.size(), 0) == -1)
			std::cout << "failed to send mem size" << std::endl;
		rc = 0;
	} else if (strstr(cmd, "path") != 0) {
		char cwd[100] = {0};
		getcwd(cwd, sizeof(cwd));
		if(send(sess->sock, cwd, strlen(cwd) * sizeof(char), 0) == -1)
			std::cout << "failed to send working directory" << std::endl;
		rc = 0;
	} else if (strstr(cmd, "enumerate") != 0) {
		std::string processList;
		getProcessList(processList);
		std::cout << processList;
		if(send(sess->sock, processList.c_str(), processList.size(), 0) == -1)
			std::cout << "failed to send processList" << std::endl;
		rc = 0;
	} else {
		char unknown[] = "unknown";
		if(send(sess->sock, unknown, strlen(unknown), 0) == -1)
			std::cout << "failed to send unknown" << std::endl;
	}

	std::cout << sess->sock << ": " << cmd << std::endl;

	return rc;
}

void *server::sessionHandler(void *sessParam)
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

	if (maxSessions > server::kMaxSessions) {
		std::cout << "max sessions over " << server::kMaxSessions << std::endl;
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
		pthread_t tid;

		std::cout << "Waiting..." << std::endl;
		if((sessionSock = accept(listenSock, (struct sockaddr*)&remote, &sockLen)) == -1) {
			if (serverRunning)
				std::cout << "Failed accept" << std::endl;
			break;
		}

		sess.s = this;
		sess.sock = sessionSock;
		if (pthread_create(&tid, NULL, sessionHandler, &sess)) {
			std::cout << "Failed to create session handler" << std::endl;
			break;
		}

		sessions.push_back(tid);
		sessionCount++;

		std::cout << "Created " << sessionCount << "/" <<  maxSessions << "sessions" << std::endl;
	}

	std::vector<pthread_t>::iterator it;
	if (!serverRunning)
		for (it = sessions.begin(); it != sessions.end(); it++)
			pthread_cancel(*it);

	for (it = sessions.begin(); it != sessions.end(); it++)
		pthread_join(*it, NULL);

	std::cout << "server exit" << std::endl;
	return 0;
}


