#include <sys/resource.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "utils.hpp"

void sysUtils::getMemorySize(std::string &memSize)
{
	struct rusage r_usage;
	getrusage(RUSAGE_SELF,&r_usage);
	std::ostringstream m;
	m << r_usage.ru_maxrss;
	memSize = m.str();
}

void sysUtils::getProcessList(std::string &data)
{
	std::system("ps aux > proc_list.txt" );
	//std::cout << std::ifstream("proc_list.txt").rdbuf();
	std::getline(std::ifstream("proc_list.txt"), data, '\0');
	return;
}
