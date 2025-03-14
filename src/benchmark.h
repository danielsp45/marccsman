#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <map>
#include <string>

#include "result.h"
#include "options.h"
#include "kvstore.h"
#include "stats.h"

enum WriteMode { RANDOM, SEQUENTIAL };

// Define the types of distributions you want.
enum class DistributionType {
    Fixed,
    Uniform,
    Normal,
    Zipfian,
    Latest
};

struct ThreadState {
	int tid;
	std::unique_ptr<Stats> stats;

	ThreadState(int id) : tid(id), stats(std::make_unique<Stats>()) {}
};

class Benchmark {
public:
	Benchmark();
	~Benchmark();

	Result setup(std::unique_ptr<KVStore> kv, Options options);
	Result run();

private:
	std::unique_ptr<KVStore> kv;
	int num = 1000;
	int key_size = 16;
	int value_size = 1000;
	DistributionType distribution = DistributionType::Uniform;
	std::vector<std::string> workloads = {"fillseq"};
	int threads = 1;

	std::vector<CombinedStats> stats;

	Result parseOptions(Options options);
	Result parseWorkloads(std::string workloadsStr);
	Result getWorkloadMethod(const std::string &workload, std::function<void(ThreadState*)> &method);

	// Workload methods
	void writeSeq(ThreadState* thread);
	void writeRandom(ThreadState* thread);
	void doWrite(ThreadState* thread, WriteMode mode);
	void readRandom(ThreadState* thread);
	void YCSBA(ThreadState* thread);
	void YCSBB(ThreadState* thread);
	void YCSBC(ThreadState* thread);
	void YCSBD(ThreadState* thread);
	void YCSBE(ThreadState* thread);
};

#endif // BENCHMARK_H
