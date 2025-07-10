#include <iostream>
#include <iomanip>
#include <vector>
#include <numeric>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <cstddef>
#include <stdexcept>
#include "kaizen.h"

template<typename Func>
double measure_time(Func&& func) {
    auto timer = zen::timer();
    timer.start();
    func();
    timer.stop();
    return timer.duration<zen::timer::usec>().count() / 1000.0;
}

void print_result(const std::string& method, const std::string& memoryOrder,
                  long long sum, double timeMs) {
    std::cout << std::setw(20) << method
              << std::setw(20) << memoryOrder
              << std::setw(20) << sum
              << std::fixed << std::setprecision(2) << std::setw(20) << timeMs << "\n";
}

void atomic_sum(const std::vector<int>& data, std::atomic<long long>& total,
                std::memory_order order, unsigned int numThreads, 
                double* creation_time = nullptr, double* join_time = nullptr) {
    std::vector<std::thread> threads;
    size_t chunk = data.size() / numThreads;

    auto worker = [&data, &total, order](size_t start, size_t end) {
        long long localSum = 0;
        for (size_t i = start; i < end; ++i)
            localSum += data[i];
        total.fetch_add(localSum, order);
    };

    // Measure thread creation time
    auto creation_timer = zen::timer();
    if (creation_time)
        creation_timer.start();
    
    for (unsigned int i = 0; i < numThreads; ++i) {
        size_t start = i * chunk;
        size_t end = (i == numThreads - 1) ? data.size() : start + chunk;
        threads.emplace_back(worker, start, end);
    }
    
    if (creation_time) {
        creation_timer.stop();
        *creation_time = creation_timer.duration<zen::timer::usec>().count() / 1000.0;
    }

    // Measure join time
    auto join_timer = zen::timer();
    if (join_time)
        join_timer.start();
    
    for (auto& t : threads)
        t.join();
        
    if (join_time) {
        join_timer.stop();
        *join_time = join_timer.duration<zen::timer::usec>().count() / 1000.0;
    }
}

void reduce_sum(const std::vector<int>& data, std::vector<long long>& partialSums,
                unsigned int numThreads, double* creation_time = nullptr, double* join_time = nullptr) {
    std::vector<std::thread> threads;
    size_t chunk = data.size() / numThreads;

    auto worker = [&data, &partialSums](unsigned int tid, size_t start, size_t end) {
        for (size_t i = start; i < end; ++i)
            partialSums[tid] += data[i];
    };

    // Measure thread creation time
    auto creation_timer = zen::timer();
    if (creation_time) creation_timer.start();
    
    for (unsigned int i = 0; i < numThreads; ++i) {
        size_t start = i * chunk;
        size_t end = (i == numThreads - 1) ? data.size() : start + chunk;
        threads.emplace_back(worker, i, start, end);
    }
    
    if (creation_time) {
        creation_timer.stop();
        *creation_time = creation_timer.duration<zen::timer::usec>().count() / 1000.0;
    }

    // Measure join time
    auto join_timer = zen::timer();
    if (join_time) join_timer.start();
    
    for (auto& t : threads)
        t.join();
        
    if (join_time) {
        join_timer.stop();
        *join_time = join_timer.duration<zen::timer::usec>().count() / 1000.0;
    }
}

void single_thread_sum(const std::vector<int>& data, long long& result) {
    result = 0;
    for (int value : data)
        result += value;
}

// Thread Pool Implementation
class ThreadPool {
public:
    ThreadPool(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
            worker.join();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

void threadpool_sum(const std::vector<int>& data, std::atomic<long long>& total,
                   unsigned int numThreads) {
    ThreadPool pool(numThreads);
    size_t chunk = data.size() / numThreads;
    std::atomic<int> completed_tasks(0);
    std::mutex completion_mutex;
    std::condition_variable completion_cv;

    for (unsigned int i = 0; i < numThreads; ++i) {
        size_t start = i * chunk;
        size_t end = (i == numThreads - 1) ? data.size() : start + chunk;

        pool.enqueue([&data, &total, start, end, &completed_tasks, &completion_mutex, &completion_cv, numThreads]() {
            long long localSum = 0;
            for (size_t j = start; j < end; ++j) {
                localSum += data[j];
            }
            total.fetch_add(localSum, std::memory_order_relaxed);

            // Signal completion
            {
                std::lock_guard<std::mutex> lock(completion_mutex);
                completed_tasks.fetch_add(1);
            }
            completion_cv.notify_one();
        });
    }

    // Wait for all tasks to complete
    std::unique_lock<std::mutex> lock(completion_mutex);
    completion_cv.wait(lock, [&completed_tasks, numThreads]() {
        return completed_tasks.load() >= static_cast<int>(numThreads);
    });
}

void benchmark_thread_scaling(const std::vector<int>& data) {
    std::cout << "\n=== Thread Scaling Analysis ===\n";
    std::cout << std::left << std::setw(10) << "Threads"
              << std::setw(20) << "Atomic Sum (ms)"
              << std::setw(20) << "Reduce Sum (ms)"
              << std::setw(22) << "ThreadPool Sum (ms)"
              << std::setw(22) << "Thread Overhead (ms)" << "\n";
    std::cout << zen::repeat("-", 94) << "\n";

    std::vector<unsigned int> threadCounts = {1, 2, 4, 8, 12, 16};
    unsigned int maxThreads = std::thread::hardware_concurrency();
    if (maxThreads > 16) {
        threadCounts.push_back(maxThreads);
    }

    for (unsigned int numThreads : threadCounts) {
        if (numThreads > maxThreads && numThreads != maxThreads) continue;

        // Atomic sum benchmark with thread timing
        std::atomic<long long> atomicTotal(0);
        double atomicCreationTime = 0, atomicJoinTime = 0;
        double atomicTime = measure_time([&]() {
            atomic_sum(data, atomicTotal, std::memory_order_relaxed, numThreads, &atomicCreationTime, &atomicJoinTime);
        });

        // Reduce sum benchmark with thread timing
        std::vector<long long> partialSums(numThreads, 0);
        double reduceCreationTime = 0, reduceJoinTime = 0;
        double reduceTime = measure_time([&]() {
            reduce_sum(data, partialSums, numThreads, &reduceCreationTime, &reduceJoinTime);
        });

        // ThreadPool sum benchmark
        std::atomic<long long> poolTotal(0);
        double poolTime = measure_time([&]() {
            threadpool_sum(data, poolTotal, numThreads);
        });

        // Calculate average thread overhead (creation + join)
        double avgThreadOverhead = (atomicCreationTime + atomicJoinTime + reduceCreationTime + reduceJoinTime) / 2.0;

        std::cout << std::setw(10) << numThreads
                  << std::fixed << std::setprecision(2)
                  << std::setw(20) << atomicTime
                  << std::setw(20) << reduceTime
                  << std::setw(22) << poolTime
                  << std::setw(22) << avgThreadOverhead << "\n";
    }
}

void benchmark_workload_scaling() {
    std::cout << "\n=== Workload Scaling Analysis ===\n";
    std::cout << std::left << std::setw(15) << "Data Size"
              << std::setw(15) << "Threads (ms)"
              << std::setw(18) << "ThreadPool (ms)"
              << std::setw(15) << "Speedup Ratio" << "\n";
    std::cout << zen::repeat("-", 65) << "\n";

    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0)
        numThreads = 4;

    std::vector<size_t> workloadSizes = {1000000, 5000000, 10000000, 50000000, 100000000};

    for (size_t dataSize : workloadSizes) {
        std::vector<int> testData(dataSize);
        std::iota(testData.begin(), testData.end(), 1);

        // Regular threads
        std::atomic<long long> threadsTotal(0);
        double threadsTime = measure_time([&]() {
            atomic_sum(testData, threadsTotal, std::memory_order_relaxed, numThreads);
        });

        // ThreadPool
        std::atomic<long long> poolTotal(0);
        double poolTime = measure_time([&]() {
            threadpool_sum(testData, poolTotal, numThreads);
        });

        double speedupRatio = threadsTime / poolTime;

        std::cout << std::setw(15) << dataSize
                  << std::fixed << std::setprecision(2)
                  << std::setw(15) << threadsTime
                  << std::setw(18) << poolTime
                  << std::setw(15) << speedupRatio << "\n";
    }
}

int main(int argc, char** argv) {
    zen::cmd_args args(argv, argc);
    size_t dataSize = 100000000;
    if (args.is_present("--n")) {
        auto n = std::stoi(args.get_options("--n")[0]);
        if (n > 0)
            dataSize = n;
    }

    std::vector<int> data(dataSize);
    std::iota(data.begin(), data.end(), 1);

    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 2;

    std::cout << "Thread Count: " << numThreads << "\n\n";
    
    // Original benchmark
    std::cout << "=== Basic Performance Comparison ===\n";
    std::cout << std::left << std::setw(20) << "Method"
              << std::setw(20) << "Memory Order"
              << std::setw(20) << "Sum"
              << std::setw(20) << "Time (ms)" << "\n";
    std::cout << zen::repeat("-", 80) << "\n";

    for (auto order : {std::memory_order_relaxed, std::memory_order_seq_cst}) {
        std::atomic<long long> total(0);
        double time = measure_time([&]() {
            atomic_sum(data, total, order, numThreads);
        });
        print_result("Atomic Sum",
                     order == std::memory_order_relaxed ? "relaxed" : "seq_cst",
                     total.load(),
                     time);
    }

    std::vector<long long> partialSums(numThreads, 0);
    double reduce_time = measure_time([&]() {
        reduce_sum(data, partialSums, numThreads);
    });

    long long reduceResult = 0;
    for (auto sum : partialSums) {
        reduceResult += sum;
    }
    print_result("Reduce Sum", "N/A", reduceResult, reduce_time);

    // ThreadPool benchmark
    std::atomic<long long> poolTotal(0);
    double pool_time = measure_time([&]() {
        threadpool_sum(data, poolTotal, numThreads);
    });
    print_result("ThreadPool Sum", "N/A", poolTotal.load(), pool_time);

    long long singleThreadResult = 0;
    double single_thread_time = measure_time([&]() {
        single_thread_sum(data, singleThreadResult);
    });
    print_result("Single-Threaded", "N/A", singleThreadResult, single_thread_time);

    // Advanced benchmarks
    benchmark_thread_scaling(data);
    benchmark_workload_scaling();

    return 0;
}
