#include <pthread.h>
#include <vector>

class server {

	private:
		char *sockPath;
		int maxSessions;
		std::vector<pthread_t> sessions;
		//int serverRunning;
		//int handleCommand(char *cmd);
		//void *sessionHandler(void *sessionSockParam);
		static const size_t kMaxSessions = 10;

	public:
		server(char *_sockPath, int _maxSessions);
		//~server();
		int run();
		int serverRunning;
};

