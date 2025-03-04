#include "options.h"

Options::Options() = default;
Options::~Options() = default;

Result Options::parse(int argc, char *argv[]) {
    options_.clear();

    // Iterate over all arguments.
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        // Only process options that start with "--"
        if (arg.find("--") != 0) {
            continue;
        }

        // Remove the leading "--" and split at the '='.
        std::string option = arg.substr(2);
        size_t pos = option.find('=');
        if (pos == std::string::npos) {
            // Ignore options without '='.
            continue;
        }

        std::string key = option.substr(0, pos);
        std::string value = option.substr(pos + 1);

        if (key == "adapter") {
            adapter = value;
        } else {
            options_[key] = value;
        }
    }

    if (adapter.empty()) {
        return Result::Error("No adapter provided in the options.");
    }

    return Result::OK();
}

std::map<std::string, std::string> Options::getGlobalOptionsAsMap() {
	std::map<std::string, std::string> globalOptions;
	for (const auto &option : options_) {
		// if the option key has the adapter prefix in it, skip it
		if (option.first.find(adapter + "-") == 0) {
			continue;
		} else {
			globalOptions[option.first] = option.second;
		}
	}

	return globalOptions;
}

std::map<std::string, std::string> Options::getAdapterOptionsAsMap() {
	std::map<std::string, std::string> adapterOptions;
	for (const auto &option : options_) {
		// if the option key has the adapter prefix in it, remove the prefix and add it to the returning map
		if (option.first.find(adapter + "-") == 0) {
			// change the option key by adding random chars to the value in the option_ map
			adapterOptions[option.first.substr(adapter.size() + 1)] = option.second;
		} else {
			continue;
		}
	}

	return adapterOptions;
}

