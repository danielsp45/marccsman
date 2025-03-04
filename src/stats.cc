#include "stats.h"
#include <chrono>
#include <cmath>
#include <algorithm>

// ----------
// SimpleClock Implementation
// ----------
uint64_t SimpleClock::nowMicros() const {
    auto now = std::chrono::steady_clock::now();
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch());
    return static_cast<uint64_t>(micros.count());
}

// ----------
// Stats Implementation
// ----------
Stats::Stats()
    : clock(new SimpleClock()),
      startTime(0),
      finishTime(0),
      done(0),
      bytes(0),
      seconds(0) {
    start();
}

Stats::~Stats() {
    delete clock;
}

void Stats::start() {
    startTime = clock->nowMicros();
    finishTime = startTime;
    done = 0;
    bytes = 0;
}

void Stats::finishedSingleOp(uint64_t opBytes) {
    finishedOps(1, opBytes);
}

void Stats::finishedOps(int64_t numOps, uint64_t opBytes) {
    done += numOps;
    bytes += opBytes;
}

void Stats::stop() {
    finishTime = clock->nowMicros();
    seconds = (finishTime - startTime) * 1e-6;  // convert micros to seconds
}

void Stats::report(const std::string& benchName) const {
    uint64_t effectiveOps = (done < 1 ? 1 : done);
    double elapsed = seconds;
    double avgMicrosPerOp = (finishTime - startTime) / static_cast<double>(effectiveOps);
    double opsPerSec = done / elapsed;
    double mbPerSec = (bytes / 1048576.0) / elapsed;
    printf("%-12s : %.3f micros/op, %.0f ops/sec, %.1f mb/sec, total ops: %llu\n",
           benchName.c_str(), avgMicrosPerOp, opsPerSec, mbPerSec, done);
}

uint64_t Stats::getStart() const { return startTime; }
uint64_t Stats::getFinish() const { return finishTime; }
uint64_t Stats::getOps() const { return done; }
uint64_t Stats::getBytes() const { return bytes; }
double Stats::getSeconds() const { return seconds; }

void Stats::merge(const Stats& other) {
    if (other.startTime < startTime) {
        startTime = other.startTime;
    }
    if (other.finishTime > finishTime) {
        finishTime = other.finishTime;
    }
    done += other.done;
    bytes += other.bytes;
    seconds = (finishTime - startTime) * 1e-6;
}

// ----------
// CombinedStats Implementation
// ----------
CombinedStats::CombinedStats(const std::string& benchName)
    : benchName(benchName), throughputOps(), throughputMB() {}

CombinedStats::~CombinedStats() {}

void CombinedStats::addStats(const Stats& stat) {
    double elapsed = (stat.getFinish() - stat.getStart()) * 1e-6;
    if (elapsed <= 0.0) elapsed = 1.0;
    throughputOps.push_back(stat.getOps() / elapsed);
    if (stat.getBytes() > 0) {
        double mbs = stat.getBytes() / 1048576.0;
        throughputMB.push_back(mbs / elapsed);
    }
}

double CombinedStats::calcAvg(const std::vector<double>& data) const {
    double sum = 0;
    for (double d : data) {
        sum += d;
    }
    return sum / data.size();
}

void CombinedStats::reportFinal() const {
    if (throughputOps.empty()) return;
    double avgOps = calcAvg(throughputOps);
    double avgMB = throughputMB.empty() ? 0.0 : calcAvg(throughputMB);
    printf("%s : %.0f ops/sec, %.1f MB/sec\n",
           benchName.c_str(), avgOps, avgMB);
}
