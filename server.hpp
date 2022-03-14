#include <pthread.h>
#include <vector>
#include <cstring>

class server {

	private:
		std::string sockPath;
		int maxSessions;
		std::vector<pthread_t> sessions;
		//int serverRunning;
		//int handleCommand(char *cmd);
		//void *sessionHandler(void *sessionSockParam);
		static const size_t kMaxSessions = 10;

	public:
		server(std::string _sockPath, int _maxSessions);
		//~server();
		int run();
		int serverRunning;
};

