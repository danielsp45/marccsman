#include "rocksdb_adapter.h"
#include <iostream>

// Explicitly define the default constructor.
RocksDBAdapter::RocksDBAdapter() = default;
// Explicitly define the destructor.
RocksDBAdapter::~RocksDBAdapter() = default;

// Dummy initialization: simply prints a message.
Result RocksDBAdapter::init(std::map<std::string, std::string> options) {
    std::cout << "[RocksDBAdapter] init() called." << std::endl;
    Result s = parseOptions(options);

	return s;
}

// Dummy put: prints the key and value.
Result RocksDBAdapter::put(const std::string &key, const std::string &value) {
    std::cout << "[RocksDBAdapter] put() called with key: " << key 
              << " and value: " << value << std::endl;
    return Result::OK();
}

// Dummy get: prints the key and simulates a successful get.
Result RocksDBAdapter::get(const std::string &key) {
    std::cout << "[RocksDBAdapter] get() called with key: " << key << std::endl;
    // In a real adapter you would fetch the value and perhaps print or return it.
    return Result::OK();
}

// Dummy remove: prints the key being removed.
Result RocksDBAdapter::remove(const std::string &key) {
    std::cout << "[RocksDBAdapter] remove() called with key: " << key << std::endl;
    return Result::OK();
}

// Dummy scan: prints the range being scanned.
Result RocksDBAdapter::scan(const std::string &start, const std::string &end) {
    std::cout << "[RocksDBAdapter] scan() called from: " << start 
              << " to: " << end << std::endl;
    return Result::OK();
}

// Dummy parseOptions: simply prints the options map.
Result RocksDBAdapter::parseOptions(const std::map<std::string, std::string> &opts) {
    std::cout << "[RocksDBAdapter] parseOptions() called." << std::endl;
    for (const auto &pair : opts) {
        std::cout << "  Option: " << pair.first << " = " << pair.second << std::endl;
    }
    return Result::OK();
}
