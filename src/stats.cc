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
    : clock_(new SimpleClock()),
      startTime_(0),
      finishTime_(0),
      done_(0),
      bytes_(0),
      seconds_(0) {
    start();
}

Stats::~Stats() {
    delete clock_;
}

void Stats::start() {
    startTime_ = clock_->nowMicros();
	lastOpTime_ = startTime_;
	finishTime_ = 0;
	done_ = 0;
	bytes_ = 0;
	seconds_ = 0;
	opLatencies_.clear();
}


void Stats::finishedReadOp(uint64_t opBytes, bool found) {
    uint64_t now = clock_->nowMicros();
    opLatencies_.push_back(now - lastOpTime_);
    lastOpTime_ = now;
    reads_++;
    if (found) {
        found_++;
    }
}

void Stats::finishedWriteOp(uint64_t opBytes) {
    uint64_t now = clock_->nowMicros();
    opLatencies_.push_back(now - lastOpTime_);
    lastOpTime_ = now;
    writes_++;
}

void Stats::finishedDeleteOp(uint64_t opBytes) {
    uint64_t now = clock_->nowMicros();
    opLatencies_.push_back(now - lastOpTime_);
    lastOpTime_ = now;
    deletes_++;
}

void Stats::finishedOps(int64_t numOps, uint64_t opBytes) {
    done_ += numOps;
    bytes_ += opBytes;
}

void Stats::stop() {
    finishTime_ = clock_->nowMicros();
    seconds_ = (finishTime_ - startTime_) * 1e-6;  // convert micros to seconds
}

uint64_t Stats::getStart() const { return startTime_; }
uint64_t Stats::getFinish() const { return finishTime_; }
uint64_t Stats::getOps() const { return done_; }
uint64_t Stats::getBytes() const { return bytes_; }
double Stats::getSeconds() const { return seconds_; }
std::vector<double> Stats::getOpLatencies() const { return opLatencies_; }

void Stats::merge(const Stats& other) {
    if (other.startTime_ < startTime_) {
        startTime_ = other.startTime_;
    }
    if (other.finishTime_ > finishTime_) {
        finishTime_ = other.finishTime_;
    }
    done_ += other.done_;
    bytes_ += other.bytes_;
    seconds_ = (finishTime_ - startTime_) * 1e-6;
}

// ----------
// CombinedStats Implementation
// ----------
CombinedStats::CombinedStats(const std::string& benchName)
    : benchName_(benchName) {}

CombinedStats::~CombinedStats() = default;

void CombinedStats::addStats(std::unique_ptr<Stats> stat) {
    double elapsed = stat->getSeconds();
    uint64_t ops = stat->getOps();
    // Calculate throughput (ops/sec).
    double opThroughput = static_cast<double>(ops) / elapsed;
    throughputOps_.push_back(opThroughput);
    // If bytes > 0, compute MB/sec.
    if (stat->getBytes() > 0) {
        double mbPerSec = (static_cast<double>(stat->getBytes()) / 1048576.0) / elapsed;
        throughputMB_.push_back(mbPerSec);
    }
    // Append the per-operation latencies recorded in Stats.
    const auto& latencies = stat->getOpLatencies();
    opLatencies_.insert(opLatencies_.end(), latencies.begin(), latencies.end());
}

void CombinedStats::reportFinal() const {
    // Report latency-related metrics if any latencies have been recorded.
    if (!opLatencies_.empty()) {
        double avgLatency = calcAvg(opLatencies_);
        double medLatency = calcMedian(opLatencies_);
        double p90Latency = calcPercentile(opLatencies_, 90.0);
        double p99Latency = calcPercentile(opLatencies_, 99.0);
        printf("==== %s Results ====\n", benchName_.c_str());
        printf("Latency (µs):\n");
        printf("   Avg    : %.3f\n", avgLatency);
        printf("   Median : %.3f\n", medLatency);
        printf("   P90    : %.3f\n", p90Latency);
        printf("   P99    : %.3f\n", p99Latency);
    }
    // Report throughput results if available.
    if (!throughputOps_.empty()) {
        double avgOps = calcAvg(throughputOps_);
        printf("Throughput:\n");
        printf("   Avg    : %.0f ops/sec", avgOps);
        if (!throughputMB_.empty()) {
            double avgMB = calcAvg(throughputMB_);
            printf(" (%.1f MB/sec)", avgMB);
        }
        printf("\n");
    }
    printf("========================\n");
}

double CombinedStats::calcAvg(const std::vector<double>& data) const {
    double sum = 0.0;
    for (double d : data) {
        sum += d;
    }
    return sum / data.size();
}

double CombinedStats::calcStdDev(const std::vector<double>& data, double avg) const {
    double sumSq = 0.0;
    for (double d : data) {
        double diff = d - avg;
        sumSq += diff * diff;
    }
    return std::sqrt(sumSq / data.size());
}

double CombinedStats::calcPercentile(const std::vector<double>& data, double percentile) const {
    if (data.empty()) {
        return 0.0;
    }
    std::vector<double> copy = data;
    std::sort(copy.begin(), copy.end());
    // Compute the index (position) in the sorted array.
    double pos = (percentile / 100.0) * (copy.size() - 1);
    size_t idxLower = static_cast<size_t>(std::floor(pos));
    size_t idxUpper = static_cast<size_t>(std::ceil(pos));
    if (idxLower == idxUpper) {
        return copy[idxLower];
    } else {
        double fraction = pos - idxLower;
        return copy[idxLower] * (1.0 - fraction) + copy[idxUpper] * fraction;
    }
}

double CombinedStats::calcMedian(const std::vector<double>& data) const {
    return calcPercentile(data, 50.0);
}
