#include <pthread.h>
#include <vector>
#include <cstring>

class sysUtils {
	public:
		static void getMemorySize(std::string &memSize);
		static void getProcessList(std::string &data);
};

class server {

	private:
		std::string sockPath;
		int maxSessions;
		std::vector<pthread_t> sessions;
		int serverRunning;
		static int handleCommand(char *cmd, struct session *);
		static void *sessionHandler(void *sessionSockParam);
		static const size_t kMaxSessions = 10;
		static void handleSigAlarm(int sig);
	public:
		server(std::string _sockPath, int _maxSessions);
		int run();
};

