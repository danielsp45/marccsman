#include "rocksdb_adapter.h"
#include <iostream>

// Explicit default constructor.
RocksDBAdapter::RocksDBAdapter()
    : dbPtr_(nullptr) {
    // You might set some defaults here.
    dbOptions_.create_if_missing = true;
}

// Explicit destructor.
RocksDBAdapter::~RocksDBAdapter() {
    // Close the database if it was opened.
    if (dbPtr_ != nullptr) {
        delete dbPtr_;
    }
}

// Parses adapter options and stores them in member variables.
Result RocksDBAdapter::parseOptions(const std::map<std::string, std::string> &opts) {
    std::cout << "[RocksDBAdapter] parseOptions() called." << std::endl;
    // For example, we look for a "db" option to use as our database path.
    auto it = opts.find("db");
    if (it != opts.end()) {
        db_ = it->second;
    } else {
        // Use a default path if not provided.
        db_ = "testdb";
    }
    // Print all the options.
    for (const auto &pair : opts) {
        std::cout << "  Option: " << pair.first << " = " << pair.second << std::endl;
    }
    return Result::OK();
}

// Initialize the RocksDB adapter: parse options and open the DB.
Result RocksDBAdapter::init(std::map<std::string, std::string> options) {
    std::cout << "[RocksDBAdapter] init() called." << std::endl;
    // Store a copy of the options.
    options_ = options;
    // Parse options (e.g. to get the DB path).
    Result r = parseOptions(options_);
    if (!r.ok()) {
        return r;
    }
    
    // Open the RocksDB instance.
    rocksdb::Status status = rocksdb::DB::Open(dbOptions_, db_, &dbPtr_);
    if (!status.ok()) {
        std::cerr << "[RocksDBAdapter] Error opening RocksDB: " << status.ToString() << std::endl;
        return Result::Error(status.ToString());
    }
    std::cout << "[RocksDBAdapter] RocksDB opened at " << db_ << std::endl;
    return Result::OK();
}

// Implements put by calling RocksDB::Put.
Result RocksDBAdapter::put(const std::string &key, const std::string &value) {
    rocksdb::Status status = dbPtr_->Put(rocksdb::WriteOptions(), key, value);
    if (!status.ok()) {
        std::cerr << "[RocksDBAdapter] put() error: " << status.ToString() << std::endl;
        return Result::Error(status.ToString());
    }
    return Result::OK();
}

// Implements get by calling RocksDB::Get.
Result RocksDBAdapter::get(const std::string &key) {
    std::string value;
    rocksdb::Status status = dbPtr_->Get(rocksdb::ReadOptions(), key, &value);
    if (!status.ok()) {
        std::cerr << "[RocksDBAdapter] get() error: " << status.ToString() << std::endl;
        return Result::Error(status.ToString());
    }
    std::cout << "[RocksDBAdapter] get() key: " << key << " value: " << value << std::endl;
    return Result::OK();
}

// Implements remove (delete) by calling RocksDB::Delete.
Result RocksDBAdapter::remove(const std::string &key) {
    rocksdb::Status status = dbPtr_->Delete(rocksdb::WriteOptions(), key);
    if (!status.ok()) {
        std::cerr << "[RocksDBAdapter] remove() error: " << status.ToString() << std::endl;
        return Result::Error(status.ToString());
    }
    return Result::OK();
}

// Implements scan by iterating from the start key to the end key.
Result RocksDBAdapter::scan(const std::string &start, const std::string &end) {
    rocksdb::ReadOptions readOptions;
    std::unique_ptr<rocksdb::Iterator> it(dbPtr_->NewIterator(readOptions));
    std::cout << "[RocksDBAdapter] scan() from: " << start << " to: " << end << std::endl;
    for (it->Seek(start); it->Valid() && it->key().ToString() < end; it->Next()) {
        std::cout << "   " << it->key().ToString() << " : " << it->value().ToString() << std::endl;
    }
    if (!it->status().ok()) {
        return Result::Error(it->status().ToString());
    }
    return Result::OK();
}
