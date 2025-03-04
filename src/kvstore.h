#ifndef KVSTORE_H
#define KVSTORE_H

#include <iostream>
#include <string>
#include <map>

#include "result.h"
#include "options.h"

class KVStore {
public:
	virtual ~KVStore() = default;
    virtual Result init(std::map<std::string, std::string> options) = 0;
    virtual Result put(const std::string &key, const std::string &value) = 0;
    virtual Result get(const std::string &key) = 0;
    virtual Result remove(const std::string &key) = 0;
    virtual Result scan(const std::string &start, const std::string &end) = 0;
};

#endif // KVSTORE_H
