#include <sys/resource.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "utils.hpp"

std::string sysUtils::getMemorySize()
{
	struct rusage r_usage;
	getrusage(RUSAGE_SELF,&r_usage);
	std::ostringstream m;
	m << r_usage.ru_maxrss;
	return m.str();
}

std::string sysUtils::getProcessList()
{
	std::string data;
	std::system("ps aux > proc_list.txt" );
	std::getline(std::ifstream("proc_list.txt"), data, '\0');
	return data;
}
