#include <iostream>
#include "kvstore_factory.h"
#include "options.h"
#include "benchmark.h"
// Remove the static include of rocksdb_adapter.h if it is now in the plugin.

extern void loadPlugins(KVStoreFactory&, const std::string&); // Declaration from plugin_loader.cc

int main(int argc, char *argv[])
{
    Options options;
    Result r = options.parse(argc, argv);
    if (!r.ok()) {
        std::cerr << "Error: " << r.message() << std::endl;
        return 1;
    }

    KVStoreFactory& factory = KVStoreFactory::instance();
    // Load all plugins from a specified directory (e.g., "./adapters")
    loadPlugins(factory, "./adapters");

    std::unique_ptr<KVStore> kv = factory.create(options.adapter);
    Benchmark benchmark;
    r = benchmark.setup(std::move(kv), options);
    if (!r.ok()) {
        std::cerr << "Error: " << r.message() << std::endl;
        return 1;
    }

    r = benchmark.run();
    if (!r.ok()) {
        std::cerr << "Error: " << r.message() << std::endl;
        return 1;
    }

    return 0;
}
