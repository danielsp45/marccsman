#include "kvstore_factory.h"
#include <stdexcept>

// Returns a reference to the single global instance of KVStoreFactory.
// This implements the Singleton pattern.
KVStoreFactory& KVStoreFactory::instance() {
    static KVStoreFactory instance; // Guaranteed to be initialized once
    return instance;
}

// Registers a new adapter creation function under the specified name.
void KVStoreFactory::registerAdapter(const std::string& name, CreatorFunc creator) {
    registry[name] = creator;
}

// Creates an adapter instance by looking up the provided name in the registry.
// If the adapter is not found, this function throws a runtime_error.
std::unique_ptr<KVStore> KVStoreFactory::create(const std::string& name) const {
    auto it = registry.find(name);
    if (it != registry.end()) {
        // Call the creator function to instantiate the adapter.
        return (it->second)();
    } else {
        throw std::runtime_error("Adapter not found: " + name);
    }
}
