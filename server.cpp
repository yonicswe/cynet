
#include <sys/stat.h>
#include <iostream>
#include <cstring>
#include "server.hpp"

int main(int argc, char *argv[])
{

	if (argc < 2)
		std::cout << "usage: ./server <socket path> <max sessions>" << std::endl;

	std::string sockPath(argv[1]);
	int maxSessions = atoi(argv[2]);

	struct stat statBuffer;
	if (stat(sockPath.c_str(), &statBuffer)) {
		std::cout << "file " << sockPath << " does not exist" << std::endl;
		return -1;
	}

	server s(sockPath, maxSessions);
	s.run();

	return 0;
}
