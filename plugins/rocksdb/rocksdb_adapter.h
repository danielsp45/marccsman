#ifndef ROCKSDB_ADAPTER_H
#define ROCKSDB_ADAPTER_H

#include <map>
#include <string>
#include <memory>
#include "kvstore.h"
#include "kvstore_factory.h"
#include "result.h"

// Include RocksDB headers
#include "rocksdb/db.h"
#include "rocksdb/options.h"

class RocksDBAdapter : public KVStore {
public:
    RocksDBAdapter();
    ~RocksDBAdapter();  // explicit destructor

    Result init(std::map<std::string, std::string> options) override;
    Result put(const std::string &key, const std::string &value) override;
    Result get(const std::string &key) override;
    Result remove(const std::string &key) override;
    Result scan(const std::string &start, const std::string &end) override;

private:
    // Options provided from the benchmark tool.
    std::map<std::string, std::string> options_;
    // Database path (and optionally column family if needed).
    std::string db_;
    std::string cf_;

    // RocksDB internal objects.
    rocksdb::DB* dbPtr_;
    rocksdb::Options dbOptions_;

    // Parse options coming from the benchmark tool.
    Result parseOptions(const std::map<std::string, std::string> &options);
};

#endif // ROCKSDB_ADAPTER_H
