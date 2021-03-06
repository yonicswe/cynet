#ifndef __SERVER_HPP__
#define __SERVER_HPP__
#include <pthread.h>
#include <vector>
#include <cstring>
#include <map>
#include <thread>
#include "utils.hpp"

class server {

	private:
		std::string sockPath;
		int maxSessions;
		std::vector<std::thread> sessions;
		int serverRunning;
		static int handleCommand(char *cmd, struct session *);
		static void sessionHandler(void *sessionSockParam);
		static const size_t kMaxSessions = 10;
		static void handleSigAlarm(int sig);
		static std::map<std::string, int> commandTable;

	public:
		server(std::string _sockPath, int _maxSessions);
		int run();
};
#endif // __SERVER_HPP__
