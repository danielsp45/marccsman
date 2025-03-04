#ifndef OPTIONS_H
#define OPTIONS_H

#include <map>
#include <string>

#include "result.h"

class Options {
public:
	std::string adapter;

	Options();
	~Options();

	Result parse(int argc, char *argv[]);
	std::map<std::string, std::string> getGlobalOptionsAsMap();
	std::map<std::string, std::string> getAdapterOptionsAsMap();
private:
	std::map<std::string, std::string> options_;
};

#endif // OPTIONS_H
