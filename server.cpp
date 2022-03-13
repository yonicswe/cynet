
#include <sys/stat.h>
#include <iostream>
#include "server.hpp"
using namespace std;

int main(int argc, char *argv[])
{
	char *sockPath;
	int maxSessions;
	struct stat statBuffer;

	if (argc < 2)
		cout << "usage: ./server <socket path> <max sessions>\n";

	sockPath = argv[1];
	maxSessions = atoi(argv[2]);

	if (stat(sockPath, &statBuffer)) {
		cout << "file " << sockPath << " does not exist\n";
		return -1;
	}

	server s(sockPath, maxSessions);
	s.run();

	return 0;
}
