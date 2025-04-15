#include <iostream>
#include <iomanip>
#include <vector>
#include <numeric>
#include <thread>
#include <atomic>
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
                std::memory_order order, unsigned int numThreads) {
    std::vector<std::thread> threads;
    size_t chunk = data.size() / numThreads;

    auto worker = [&data, &total, order](size_t start, size_t end) {
        long long localSum = 0;
        for (size_t i = start; i < end; ++i)
            localSum += data[i];
        total.fetch_add(localSum, order);
    };

    for (unsigned int i = 0; i < numThreads; ++i) {
        size_t start = i * chunk;
        size_t end = (i == numThreads - 1) ? data.size() : start + chunk;
        threads.emplace_back(worker, start, end);
    }

    for (auto& t : threads)
        t.join();
}

void reduce_sum(const std::vector<int>& data, std::vector<long long>& partialSums,
                unsigned int numThreads) {
    std::vector<std::thread> threads;
    size_t chunk = data.size() / numThreads;

    auto worker = [&data, &partialSums](unsigned int tid, size_t start, size_t end) {
        for (size_t i = start; i < end; ++i)
            partialSums[tid] += data[i];
    };

    for (unsigned int i = 0; i < numThreads; ++i) {
        size_t start = i * chunk;
        size_t end = (i == numThreads - 1) ? data.size() : start + chunk;
        threads.emplace_back(worker, i, start, end);
    }

    for (auto& t : threads)
        t.join();
}

void single_thread_sum(const std::vector<int>& data, long long& result) {
    result = 0;
    for (int value : data)
        result += value;
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

    std::cout << std::left << std::setw(20) << "Method"
              << std::setw(20) << "Memory Order"
              << std::setw(20) << "Sum"
              << std::setw(20) << "Time (ms)" << "\n";
    std::cout << std::string(80, '-') << "\n";

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

    long long singleThreadResult = 0;
    double single_thread_time = measure_time([&]() {
        single_thread_sum(data, singleThreadResult);
    });
    print_result("Single-Threaded", "N/A", singleThreadResult, single_thread_time);

    return 0;
}
