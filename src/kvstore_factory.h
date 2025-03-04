#ifndef KVSTORE_FACTORY_H
#define KVSTORE_FACTORY_H

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "kvstore.h"

class KVStoreFactory {
public:
    using CreatorFunc = std::function<std::unique_ptr<KVStore>()>;

    static KVStoreFactory& instance();

    void registerAdapter(const std::string& name, CreatorFunc creator);
    std::unique_ptr<KVStore> create(const std::string& name) const;

private:
    KVStoreFactory() = default;
    std::map<std::string, CreatorFunc> registry;
};

#endif // KVSTORE_FACTORY_H
