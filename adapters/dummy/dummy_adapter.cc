#include "dummy_adapter.h"

Result DummyAdapter::init(std::map<std::string, std::string> options) {
    std::cout << "DummyAdapter::init called" << std::endl;
    return Result::OK();
}

Result DummyAdapter::put(const std::string& key, const std::string& value) {
    std::cout << "DummyAdapter::put called. Key: " << key << ", Value: " << value << std::endl;
    return Result::OK();
}

Result DummyAdapter::get(const std::string& key) {
    std::cout << "DummyAdapter::get called. Key: " << key << std::endl;
    return Result::OK();
}

Result DummyAdapter::remove(const std::string& key) {
    std::cout << "DummyAdapter::remove called. Key: " << key << std::endl;
    return Result::OK();
}

Result DummyAdapter::scan(const std::string& start, const std::string& end) {
    std::cout << "DummyAdapter::scan called. Start: " << start << ", End: " << end << std::endl;
    return Result::OK();
}