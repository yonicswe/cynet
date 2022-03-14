#include <pthread.h>
#define MAX_SESSIONS 10

class server {

	private:
		char *sockPath;
		int maxSessions;
		pthread_t sessions[MAX_SESSIONS];
		//int serverRunning;
		//int handleCommand(char *cmd);
		//void *sessionHandler(void *sessionSockParam);

	public:
		server(char *_sockPath, int _maxSessions);
		//~server();
		int run();
		int serverRunning;
};

