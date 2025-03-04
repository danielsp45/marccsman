#ifndef STATS_H
#define STATS_H

#include <cstdio>
#include <vector>
#include <cstdint>
#include <string>

// --------------------------
// SimpleClock: a minimal clock class
// --------------------------
class SimpleClock {
public:
    // Returns current time in microseconds.
    uint64_t nowMicros() const;
};

//
// Stats: Per-thread statistics
//
class Stats {
public:
    Stats();
    ~Stats();

    // Initialize or reset stats.
    void start();
    // Record a single operation.
    void finishedSingleOp(uint64_t opBytes);
    // Record a batch of operations.
    void finishedOps(int64_t numOps, uint64_t opBytes);
    // Finalize stats and compute elapsed time.
    void stop();
    // Report the statistics to stdout.
    void report(const std::string& benchName) const;

    // Getters to allow merging stats.
    uint64_t getStart() const;
    uint64_t getFinish() const;
    uint64_t getOps() const;
    uint64_t getBytes() const;
    double getSeconds() const;
	std::vector<double> getOpLatencies() const;

    // Merge another Stats object (for combining per-thread results).
    void merge(const Stats& other);

private:
    SimpleClock* clock_;
    uint64_t startTime_;
	uint64_t lastOpTime_;
    uint64_t finishTime_;
    uint64_t done_;   // total operations
    uint64_t bytes_;  // total bytes processed
    double seconds_;
	// store individual operation latencies
	std::vector<double> opLatencies_;
};

//
// CombinedStats: Aggregates stats from multiple threads.
//
class CombinedStats {
public:
    CombinedStats(const std::string& benchName);
    ~CombinedStats();

    void addStats(std::unique_ptr<Stats> stat);
    void reportFinal() const;

private:
    // Helper functions for latency statistics:
    double calcAvg(const std::vector<double>& data) const;
    double calcStdDev(const std::vector<double>& data, double avg) const;
    double calcPercentile(const std::vector<double>& data, double percentile) const;
    double calcMedian(const std::vector<double>& data) const;

    std::vector<double> throughputOps_;   // Ops/sec per Stats object.
    std::vector<double> throughputMB_;    // MB/sec per Stats object.
    std::vector<double> opLatencies_;     // Combined per-operation latencies (in microseconds).
    std::string benchName_;               // Benchmark name.
};

#endif // STATS_H
