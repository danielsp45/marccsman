
#include <thread>

#include "benchmark.h"

// define the benchmark options list
const std::vector<std::string> supportedWorkloads = {
	"fillseq",
	"fillrandom"
	"ycsba",
	"readrandom"
};

Benchmark::Benchmark() = default;
Benchmark::~Benchmark() = default;

Result Benchmark::setup(std::unique_ptr<KVStore> kvstore, Options options) {
	kv = std::move(kvstore);
	auto r = parseOptions(options);
	if (!r.ok()) {
		return r;
	}

	auto adapterOptions = options.getAdapterOptionsAsMap();
	return kv->init(adapterOptions);
}

Result Benchmark::run() {
	// for each workload, run the benchmark
	for (const auto &workload : workloads) {
		std::function<void(ThreadState*)> method;
		auto r = getWorkloadMethod(workload, method);
		if (!r.ok()) {
			return r;
		}

		std::vector<std::thread> t;
		std::vector<ThreadState*> workerStates;
		for (int i = 0; i < threads; i++) {
			auto *state = new ThreadState{i};
			workerStates.push_back(state);
			t.push_back(std::thread(method, state));
		}

		for (auto &thread : t) {
			thread.join();
		}

		// finally, aggregate results into CombinedStats
		auto combinedStats = CombinedStats(workload);
		for (const auto &state : workerStates) {
			combinedStats.addStats(state->stats);
			delete state;
		}

		// save the combined stats
		stats.push_back(combinedStats);
	}

	// print the results
	for (const auto &stat : stats) {
		stat.reportFinal();
	}

	return Result::OK();
}


Result Benchmark::parseWorkloads(std::string workloadsStr) {
	// clear the current workloads vector
	workloads.clear();
	size_t pos = 0;
	while ((pos = workloadsStr.find(",")) != std::string::npos) {
		std::string token = workloadsStr.substr(0, pos);
		if (!token.empty()) {
			// check if the workload is supported
			if (std::find(supportedWorkloads.begin(), supportedWorkloads.end(), token) == supportedWorkloads.end()) {
				return Result::Error("Unsupported workload: " + token);
			} else {
				workloads.push_back(token);
			}
		}
		workloadsStr.erase(0, pos + 1);
	}
	// Add the last token, if any
	if (!workloadsStr.empty()) {
		if (std::find(supportedWorkloads.begin(), supportedWorkloads.end(), workloadsStr) == supportedWorkloads.end()) {
			return Result::Error("Unsupported workload: " + workloadsStr);
		}
		workloads.push_back(workloadsStr);
	}
	return Result::OK();

}

Result Benchmark::parseOptions(Options options) {
	auto globalOptions = options.getGlobalOptionsAsMap();

	for (const auto &option : globalOptions) {
		if (option.first == "num") {
			num = std::stoi(option.second);
		} else if (option.first == "key_size") {
			key_size = std::stoi(option.second);
		} else if (option.first == "value_size") {
			value_size = std::stoi(option.second);
		} else if (option.first == "workload") {
			// clear the current workloads vector
			auto r = parseWorkloads(option.second);
			if (!r.ok()) {
				return r;
			}
		} else if (option.first == "threads") {
			threads = std::stoi(option.second);
		} else {
			return Result::Error("Unknown option: " + option.first);
		}
	}

	return Result::OK();
}

Result Benchmark::getWorkloadMethod(const std::string &workload, std::function<void(ThreadState*)> &method) {
    if (workload == "fillseq") {
        method = [this](ThreadState* thread) { writeSeq(thread); };
    } else if (workload == "fillrandom") {
        method = [this](ThreadState* thread) { writeRandom(thread); };
    } else if (workload == "readrandom") {
        method = [this](ThreadState* thread) { readRandom(thread); };
    } else {
        return Result::Error("Unknown workload: " + workload);
    }
    return Result::OK();
}

void Benchmark::writeSeq(ThreadState* thread) {
	thread->stats.start();
	doWrite(thread, WriteMode::SEQUENTIAL);
	thread->stats.stop();
}

void Benchmark::writeRandom(ThreadState* thread) {
	thread->stats.start();
	doWrite(thread, WriteMode::RANDOM);
	thread->stats.stop();
}

void Benchmark::doWrite(ThreadState* thread, WriteMode mode) {
	std::cout << "Benchmark::doWrite() called." << std::endl;
	for (int i = 0; i < num; i++) {
		// std::string key = std::to_string(i);
		// std::string value = std::string(value_size, 'a');
		std::string key = "key";
		std::string value = "value";
		if (mode == WriteMode::RANDOM) {
			key = std::to_string(rand() % num);
		}
		kv->put(key, value);
		uint64_t size = key.size() + value.size();
		thread->stats.finishedSingleOp(size);
	}
}

void Benchmark::readRandom(ThreadState* thread) {
	std::cout << "Benchmark::readRandom() called." << std::endl;
}
