#include <dlfcn.h>
#include <dirent.h>
#include <iostream>
#include "kvstore_factory.h"

using RegisterFunc = void (*)(KVStoreFactory&);

void loadPlugins(KVStoreFactory& factory, const std::string& pluginDir) {
    DIR* dir = opendir(pluginDir.c_str());
    if (!dir) {
        std::cerr << "Cannot open plugin directory: " << pluginDir << std::endl;
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        // Simple check: load shared libraries ending with .so
        if (name.size() > 3 && name.substr(name.size() - 3) == ".so") {
            std::string fullPath = pluginDir + "/" + name;
            void* handle = dlopen(fullPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
            if (!handle) {
                std::cerr << "Error loading " << fullPath << ": " << dlerror() << std::endl;
                continue;
            }

            // Look for a symbol "registerAdapters"
            RegisterFunc regFunc = reinterpret_cast<RegisterFunc>(dlsym(handle, "registerAdapters"));
            if (regFunc) {
                regFunc(factory);
            } else {
                std::cerr << "No registerAdapters function in " << fullPath << std::endl;
            }
        }
    }
    closedir(dir);
}