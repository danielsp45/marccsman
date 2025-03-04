#ifndef ROCKSDB_ADAPTER_H
#define ROCKSDB_ADAPTER_H

#include <map>

#include "../../src/kvstore.h"
#include "../../src/kvstore_factory.h"
#include "../../src/result.h"

class RocksDBAdapter : public KVStore {
public:
	RocksDBAdapter();
    ~RocksDBAdapter();  // declare an explicit destructor
	Result init(std::map<std::string, std::string> options) override;
	Result put(const std::string &key, const std::string &value) override;
	Result get(const std::string &key) override;
	Result remove(const std::string &key) override;
	Result scan(const std::string &start, const std::string &end) override;

private:
	std::map<std::string, std::string> options;
	Result parseOptions(const std::map<std::string, std::string> &options);
};

#endif // ROCKSDB_ADAPTER_H
