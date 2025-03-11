#include <thread>
#include <utility>

#include "benchmark.h"
#include <random>
#include <cassert>
#include <vector>
#include <cmath>
#include <algorithm>

// define the benchmark options list
const std::vector<std::string> supportedWorkloads = {
	"fillseq",
	"fillrandom",
    "ycsba",
	"ycsbb",
	"ycsbc",
	"ycsbd",
	"ycsbe"
};

// ----------
// Distribution Implementation
// an helper class to generate random values with different distributions
// ----------

// Base class for a distribution.
class BaseDistribution {
public:
    virtual ~BaseDistribution() = default;
    virtual unsigned int Generate() = 0;
};

// Fixed distribution always returns the same value.
class FixedDistribution : public BaseDistribution {
public:
    FixedDistribution(unsigned int value) : value_(value) {}
    unsigned int Generate() override {
        return value_;
    }
private:
    unsigned int value_;
};

// Uniform distribution returns a random value between min and max.
class UniformDistribution : public BaseDistribution {
public:
    UniformDistribution(unsigned int min, unsigned int max)
        : dist_(min, max) {}
    unsigned int Generate() override {
        return dist_(gen_);
    }
private:
    std::mt19937 gen_{ std::random_device{}() };
    std::uniform_int_distribution<unsigned int> dist_;
};

// Normal distribution returns a value centered around the average with a given stddev.
// The result is clamped to the [min, max] range.
class NormalDistribution : public BaseDistribution {
public:
    NormalDistribution(unsigned int min, unsigned int max)
        : gen_(std::random_device{}()),
          dist_((min + max) / 2.0, (max - min) / 6.0), // 99.7% of values within [min, max]
          min_(min), max_(max) {}
    unsigned int Generate() override {
        unsigned int val = static_cast<unsigned int>(std::round(dist_(gen_)));
        return std::max(min_, std::min(max_, val));
    }
private:
    std::mt19937 gen_;
    std::normal_distribution<double> dist_;
    unsigned int min_;
    unsigned int max_;
};

// ZipfianDistribution generates integers in [min, max] following a Zipfian (power-law) distribution.
class ZipfianDistribution : public BaseDistribution {
public:
    // min: lower bound (inclusive)
    // max: upper bound (inclusive)
    // s: exponent parameter (typically > 1)
    ZipfianDistribution(unsigned int min, unsigned int max, double s = 1.2)
        : min_(min), max_(max), s_(s), gen_(std::random_device{}()), uniDist_(0.0, 1.0)
    {
        // Precompute the cumulative distribution function (CDF)
        unsigned int N = max_ - min_ + 1;
        cdf_.resize(N);
        double sum = 0.0;
        for (unsigned int i = min_; i <= max_; i++) {
            sum += 1.0 / std::pow(static_cast<double>(i), s_);
            cdf_[i - min_] = sum;
        }
        // Normalize all values in the CDF so that the last element equals 1.
        for (double &val : cdf_) {
            val /= sum;
        }
    }
    
    // Generate returns a number in [min_, max_] according to the Zipfian distribution.
    unsigned int Generate() override {
        double u = uniDist_(gen_);
        // Binary search for the location in the CDF.
        auto it = std::lower_bound(cdf_.begin(), cdf_.end(), u);
        unsigned int index = it - cdf_.begin();
        return min_ + index;
    }
    
private:
    unsigned int min_;
    unsigned int max_;
    double s_;
    std::vector<double> cdf_;
    std::mt19937 gen_;
    std::uniform_real_distribution<double> uniDist_;
};

// in this distribution, the latest values are the most popular
class LatestDistribution : public BaseDistribution {
public:
    // min: the lowest possible key value (for example 0)
    // max: the highest possible key value (for example, total number of keys - 1)
    // lambda: controls how steep the decay is (a higher value makes keys even more biased toward the max)
    LatestDistribution(unsigned int min, unsigned int max, double lambda = 1.0)
        : min_(min), max_(max), lambda_(lambda), gen_(std::random_device{}()),
          expDist_(lambda) {}

    unsigned int Generate() override {
        // Generate an exponential value x. Since exp(x) decays quickly, most x will be small.
        double x = expDist_(gen_);
        // Convert it into a number u in (0,1] by using the exponential decay.
        double u = std::exp(-x);
        // Now, use u to bias the key toward the high end.
        // When u is near 1, the key is near max; when u is small, the key is lower.
        unsigned int key = max_ - static_cast<unsigned int>(u * (max_ - min_));
        return key;
    }
    
private:
    unsigned int min_;
    unsigned int max_;
    double lambda_;
    std::mt19937 gen_;
    std::exponential_distribution<double> expDist_;
};

// ----------
// RandomGenerator Implementation
// an helper class to generate random values
// ----------

// The RandomGenerator class generates random slices from pre-generated data.
class RandomGenerator {
public:
    // The constructor selects the distribution type and pre-fills the data buffer.
    // Parameters:
    // - distType: which distribution to use (Fixed, Uniform, or Normal).
    // - fixedSize: the fixed size to use if using DistributionType::Fixed.
    // - minSize, maxSize: the minimum and maximum lengths for random values.
    // - compressionRatio: (optional) a value to mimic compressibility. In this example,
    //   we simply ignore it, but you could use it to generate more repetitive data.
    RandomGenerator(DistributionType distType,
                    unsigned int fixedSize,
                    unsigned int minSize,
                    unsigned int maxSize
                    )
        : pos_(0)
    {
        switch (distType) {
            case DistributionType::Fixed:
                dist_ = std::make_unique<FixedDistribution>(fixedSize);
                break;
            case DistributionType::Normal:
                dist_ = std::make_unique<NormalDistribution>(minSize, maxSize);
                break;
            case DistributionType::Zipfian:
                dist_ = std::make_unique<ZipfianDistribution>(minSize, maxSize);
                break;
            case DistributionType::Uniform:
            default:
                dist_ = std::make_unique<UniformDistribution>(minSize, maxSize);
                break;
        }
        // Ensure our data buffer is large enough.
        // We choose 1MB or maxSize, whichever is larger.
        unsigned int targetSize = std::max(1048576u, maxSize);
        data_.reserve(targetSize);
        // For simplicity, we fill the buffer with random printable ASCII characters.
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<int> charDist(32, 126);
        while (data_.size() < targetSize) {
            data_.push_back(static_cast<char>(charDist(gen)));
        }
    }

    // Generates a string of exactly len characters from the pre-filled data.
    std::string Generate(unsigned int len) {
        assert(len <= data_.size());
        if (pos_ + len > data_.size()) {
            pos_ = 0; // Wrap around if needed.
        }
        std::string slice = data_.substr(pos_, len);
        pos_ += len;
        return slice;
    }

    // Generates a string using the current distribution to decide the length.
    std::string Generate() {
        unsigned int len = dist_->Generate();
        return Generate(len);
    }

private:
    std::string data_;
    size_t pos_;
    std::unique_ptr<BaseDistribution> dist_;
};

// ----------
// Benchmark Implementation
// ----------

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
			combinedStats.addStats(std::move(state->stats));
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
		} else if (option.first == "distribution") {
            if (option.second == "normal") {
                distribution = DistributionType::Normal;
            } else if (option.second == "zipfian") {
                distribution = DistributionType::Zipfian;
            } else if (option.second == "uniform") {
                distribution = DistributionType::Uniform;
            } else {
                return Result::Error("Unknown distribution: " + option.second);
            }
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
    } else if (workload == "ycsba") {
        method = [this](ThreadState* thread) { YCSBA(thread); };
    } else if (workload == "ycsbb") {
        method = [this](ThreadState* thread) { YCSBB(thread); };
    } else if (workload == "ycsbc") {
        method = [this](ThreadState* thread) { YCSBC(thread); };
    } else if (workload == "ycsbd") {
        method = [this](ThreadState* thread) { YCSBD(thread); };
    } else if (workload == "ycsbe") {
        method = [this](ThreadState* thread) { YCSBE(thread); };
    } else {
        return Result::Error("Unknown workload: " + workload);
    }
    return Result::OK();
}

void Benchmark::writeSeq(ThreadState* thread) {
	doWrite(thread, WriteMode::SEQUENTIAL);
}

void Benchmark::writeRandom(ThreadState* thread) {
	doWrite(thread, WriteMode::RANDOM);
}

// Helper function to pad an integer with leading zeros to match key_size.
std::string paddedKey(int number, size_t key_size) {
    std::string key = std::to_string(number);
    if (key.size() < key_size) {
        key.insert(key.begin(), key_size - key.size(), '0');
    }
    return key;
}

void Benchmark::doWrite(ThreadState* thread, WriteMode mode) {
    thread->stats->start();
    RandomGenerator rng(DistributionType::Uniform, 0, 1, key_size);
    for (int i = 0; i < num; i++) {
        std::string value = rng.Generate(value_size);
        std::string key;
        if (mode == WriteMode::RANDOM) {
            key = paddedKey(rand() % num, key_size);
        } else {
            key = paddedKey(i, key_size);
        }
        kv->put(key, value);
        uint64_t size = key.size() + value.size();
        thread->stats->finishedWriteOp(size);
    }
    thread->stats->stop();
}

// Workload A: Update heavy workload
// This workload has a mix of 50/50 reads and writes.
// An application example is a session store recording recent actions.
void Benchmark::YCSBA(ThreadState* thread) {
    // Start the timing for this thread’s workload.
    thread->stats->start();

    RandomGenerator valueGen(DistributionType::Uniform, 0, 1, value_size);
    ZipfianDistribution keyDist(0, num - 1);

    for (int i = 0; i < num; i++) {
        // Generate a key using the Zipfian distribution.
        unsigned int key_num = keyDist.Generate();
        std::string key = paddedKey(key_num, key_size);

        // Decide randomly whether to do read or update (50/50).
        if ((rand() % 100) < 50) {
            // Read operation.
            Result r = kv->get(key);
            thread->stats->finishedReadOp(key.size(), r.ok());
        } else {
            // Update operation: generate a new value and perform a put.
            std::string newValue = valueGen.Generate(value_size);
            Result r = kv->put(key, newValue);
            thread->stats->finishedWriteOp(key.size());
        }
    }

    thread->stats->stop();
}

// Workload B: Read mostly workload
// This workload has a 95/5 reads/write mix.
// Application example: photo tagging; add a tag is an update,
// but most operations are to read tags.
void Benchmark::YCSBB(ThreadState* thread) {
    // Start the timing for this thread’s workload.
    thread->stats->start();

    RandomGenerator valueGen(DistributionType::Uniform, 0, 1, value_size);
    ZipfianDistribution keyDist(0, num - 1);

    for (int i = 0; i < num; i++) {
        // Generate a key using the Zipfian distribution.
        unsigned int key_num = keyDist.Generate();
        std::string key = paddedKey(key_num, key_size);

        // Decide randomly whether to do read or update (95/5).
        if ((rand() % 100) < 95) {
            // Read operation.
            Result r = kv->get(key);
            thread->stats->finishedReadOp(key.size(), r.ok());
        } else {
            // Update operation: generate a new value and perform a put.
            std::string newValue = valueGen.Generate(value_size);
            Result r = kv->put(key, newValue);
            thread->stats->finishedWriteOp(key.size());
        }
    }

    thread->stats->stop();
}

// Workload C: Read only
// This workload is 100% read. Application example: user profile cache,
// where profiles are constructed elsewhere (e.g., Hadoop).
void Benchmark::YCSBC(ThreadState* state) {
    // Start the timing for this thread’s workload.
    state->stats->start();

    // Use Zipfian distribution for keys over the range [0, num-1]
    // ZipfianDistribution keyDist(0, num - 1, 1.2);
    ZipfianDistribution keyDist(0, num - 1);

    for (int i = 0; i < num; i++) {
        // Generate a key using the Zipfian distribution.
        unsigned int key_num = keyDist.Generate();
        std::string key = paddedKey(key_num, key_size);

        // Read operation.
        Result r = kv->get(key);
        state->stats->finishedReadOp(key.size(), r.ok());
    }

    state->stats->stop();
}

// Workload D: Read latest workload
// This workload has 95/0/5 read/update/insert mix.
// In this workload, new records are inserted, and the most recently
// inserted records are the most popular. Application example:
// user status updates; people want to read the latest.
void Benchmark::YCSBD(ThreadState* state) {
    // Start the timing for this thread’s workload.
    state->stats->start();

    // Use Zipfian distribution for keys over the range [0, num-1]
    // ZipfianDistribution keyDist(0, num - 1, 1.2);
    LatestDistribution keyDist(0, num - 1);

    for (int i = 0; i < num; i++) {
        int nextOp = rand() % 100;
        if (nextOp < 95) {
            // Read operation.
            unsigned int key_num = keyDist.Generate();
            std::string key = paddedKey(key_num, key_size);
            Result r = kv->get(key);
            state->stats->finishedReadOp(key.size(), r.ok());
        } else {
            // Update operation: generate a new value and perform a put.
            RandomGenerator valueGen(DistributionType::Uniform, 0, 1, value_size);
            unsigned int key_num = keyDist.Generate();
            std::string key = paddedKey(key_num, key_size);
            std::string newValue = valueGen.Generate(value_size);
            Result r = kv->put(key, newValue);
            state->stats->finishedWriteOp(key.size());
        }
    }

    state->stats->stop();
}

// Workload E: Short ranges.
// In this workload, short ranges of records are queried,
// instead of individual records. Application example:
// threaded conversations, where each scan is for the posts
// in a given thread (assumed to be clustered by thread id).

// Scan/insert ratio: 95/5
// Request distribution: latest
// Scan Length Distribution=uniform
// Max scan length = 100

// The insert order is hashed, not ordered. Although the scans are ordered, it
// does not necessarily follow that the data is inserted in order. For
// example, posts for thread 342 may not be inserted contiguously, but instead
// interspersed with posts from lots of other threads. The way the YCSB client
// works is that it will pick a start key, and then request a number of
// records; this works fine even for hashed insertion.
void Benchmark::YCSBE(ThreadState* state) {
    state->stats->start();

    LatestDistribution keyDist(0, num - 1, 1.2);
    UniformDistribution scanLenDist(1, 100);
    RandomGenerator valueGen(DistributionType::Uniform, 0, 1, value_size);

    for (int i = 0; i < num; i++) {
        int op = rand() % 100;
        if (op < 95) {
            // Scan operation
            unsigned int key_num = keyDist.Generate();
            std::string start_key = paddedKey(key_num, key_size);
            unsigned int scan_len = scanLenDist.Generate();
            std::string end_key = paddedKey(key_num + scan_len, key_size);

            Result r = kv->scan(start_key, end_key);
            size_t size = end_key.size() * scan_len;
            state->stats->finishedReadOp(size, r.ok());
        } else {
            // Update operation
            unsigned int key_num = keyDist.Generate();
            std::string key = paddedKey(key_num, key_size);
            std::string newValue = valueGen.Generate(value_size);
            Result r = kv->put(key, newValue);
            state->stats->finishedWriteOp(key.size());
        }
    }
    state->stats->stop();
}