
#include <sys/stat.h>
#include <iostream>
#include "server.hpp"

int main(int argc, char *argv[])
{
	char *sockPath;
	int maxSessions;
	struct stat statBuffer;

	if (argc < 2)
		std::cout << "usage: ./server <socket path> <max sessions>" << std::endl;

	sockPath = argv[1];
	maxSessions = atoi(argv[2]);

	if (stat(sockPath, &statBuffer)) {
		std::cout << "file " << sockPath << " does not exist" << std::endl;
		return -1;
	}

	server s(sockPath, maxSessions);
	s.run();

	return 0;
}
