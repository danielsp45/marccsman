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

    // Merge another Stats object (for combining per-thread results).
    void merge(const Stats& other);

private:
    SimpleClock* clock;
    uint64_t startTime;
    uint64_t finishTime;
    uint64_t done;   // total operations
    uint64_t bytes;  // total bytes processed
    double seconds;
};

//
// CombinedStats: Aggregates stats from multiple threads.
//
class CombinedStats {
public:
    CombinedStats(const std::string& benchName);
    ~CombinedStats();

    void addStats(const Stats& stat);
    void reportFinal() const;

private:
    double calcAvg(const std::vector<double>& data) const;

    std::vector<double> throughputOps;
    std::vector<double> throughputMB;
    std::string benchName;
};

#endif // STATS_H
