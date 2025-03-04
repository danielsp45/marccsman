#include <iostream>

#include "kvstore_factory.h"
#include "options.h"
#include "benchmark.h"
#include "rocksdb/rocksdb_adapter.h"  // Add this include!

void registerAdapters(KVStoreFactory& factory) {
	factory.registerAdapter("rocksdb", [](){
		return std::make_unique<RocksDBAdapter>();
	});
}

int main(int argc, char *argv[])
{
	Options options;
	Result r = options.parse(argc, argv);
	if (!r.ok())
	{	
		std::cerr << "Error: " << r.message() << std::endl;
		return 1;
	}

	KVStoreFactory& factory = KVStoreFactory::instance();
	// TODO: Improve this
	registerAdapters(factory);
	std::unique_ptr<KVStore> kv = factory.create(options.adapter);

	Benchmark benchmark;
	r = benchmark.setup(std::move(kv), options);
	if (!r.ok())
	{
		std::cerr << "Error: " << r.message() << std::endl;
		return 1;
	}

	r = benchmark.run();
	if (!r.ok())
	{
		std::cerr << "Error: " << r.message() << std::endl;
		return 1;
	}
	
    return 0;
}
