#include "dummy_adapter.h"
#include "kvstore_factory.h"
#include <memory>

extern "C" void registerAdapters(KVStoreFactory& factory) {
    factory.registerAdapter(
        "dummy", [](){
        return std::make_unique<DummyAdapter>();
    });
}