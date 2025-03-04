#ifndef DUMMY_ADAPTER_H
#define DUMMY_ADAPTER_H

#include <iostream>
#include <map>
#include <string>
#include "kvstore.h"
#include "result.h"

class DummyAdapter : public KVStore {
public:
    DummyAdapter() = default;
    ~DummyAdapter() override = default;

    Result init(std::map<std::string, std::string> options) override;
    Result put(const std::string& key, const std::string& value) override;
    Result get(const std::string& key) override;
    Result remove(const std::string& key) override;
    Result scan(const std::string& start, const std::string& end) override;
};

#endif // DUMMY_ADAPTER_H