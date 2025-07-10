# Parallel Summation Benchmark

## Overview
This C++ project demonstrates and benchmarks several parallel summation techniques by partitioning an array among multiple threads or tasks. It compares different methods for aggregating the results—using atomic operations with various memory orderings, a reduction-based approach, a thread pool implementation, and a task-based approach with `std::async`—to explore trade-offs in correctness, complexity, and speed. The project includes comprehensive performance analysis across different thread counts and workload sizes, and provides a detailed comparison of all methods, including asynchronous task-based summation.

## Problem Description
Summing large arrays efficiently is a common problem in parallel computing. The challenge lies in managing shared data access and synchronization across multiple threads. This project implements four methods for calculating the sum of a large integer array:

1. **Atomic Sum:**  
   Threads update a shared atomic variable using the `fetch_add` operation with different memory orderings:
   - **Relaxed:** Provides minimal synchronization, offering higher performance but less ordering guarantees.
   - **Sequentially Consistent (seq_cst):** Ensures a strict global order of operations, enhancing correctness at the cost of additional overhead.

2. **Reduce Sum:**  
   The array is partitioned among several threads. Each thread computes a partial sum independently, and these partial sums are then aggregated in the main thread. This approach avoids locks during computation but requires additional aggregation.

3. **ThreadPool Sum:**  
   Uses a pre-created pool of worker threads to process tasks from a queue. This eliminates thread creation/destruction overhead and provides better resource management for repeated operations.

4. **Single-Threaded Sum:**  
   A baseline method that performs the summation sequentially without multithreading, offering a point of comparison for performance metrics.

5. **Async Sum (std::async):**  
   Utilizes C++11's `std::async` to run tasks asynchronously. This method automatically manages threads and allows for easy integration of parallelism in a divide-and-conquer style.

## Example Output
An example run of the program may produce output similar to the following:

```
Thread Count: 8

=== Basic Performance Comparison ===
Method              Memory Order        Sum                 Time (ms)
--------------------------------------------------------------------------------
Atomic Sum          relaxed             5000000050000000    101.90
Atomic Sum          seq_cst             5000000050000000    95.61
Reduce Sum          N/A                 5000000050000000    701.11
ThreadPool Sum      N/A                 5000000050000000    102.51
Single-Threaded     N/A                 5000000050000000    676.95
Async Sum           N/A                 5000000050000000    229.80

=== Thread Scaling Analysis ===
Threads   Atomic Sum (ms)     Reduce Sum (ms)     ThreadPool Sum (ms)   Thread Overhead (ms)
----------------------------------------------------------------------------------------------        
1         284.77              394.00              260.02                339.38
2         147.06              648.85              144.66                397.95
4         119.55              706.12              105.75                412.83
8         116.97              645.21              116.42                381.08

=== Workload Scaling Analysis ===
Data Size      Threads (ms)   ThreadPool (ms)   Async (ms)        Speedup T/TP      Speedup T/Async   
----------------------------------------------------------------------------------------------------  
1000000        3.51           3.86              5.52              0.91              0.64
5000000        10.99          9.52              9.78              1.15              1.12
10000000       10.28          12.22             20.47             0.84              0.50
50000000       48.62          50.25             101.55            0.97              0.48
100000000      99.85          105.63            350.47            0.95              0.28
```

## Explanation of Output

### Basic Performance Comparison
- **Method:**  
  The column indicates the approach used:
  - **Atomic Sum:** Aggregates results using atomic operations with the specified memory order.
  - **Reduce Sum:** Accumulates partial sums computed by multiple threads.
  - **ThreadPool Sum:** Uses a pre-created thread pool to process summation tasks.
  - **Single-Threaded:** Uses a sequential method, serving as the performance baseline.
  - **Async Sum:** Uses asynchronous tasks to perform the summation.

- **Memory Order:**  
  Displays the memory ordering for atomic operations. For non-atomic methods such as Reduce Sum, ThreadPool Sum, Single-Threaded Sum, and Async Sum, this field is marked as `N/A`.

- **Sum:**  
  The computed total from summing the integers. In this case, the sum `5000000050000000` represents the mathematical result of summing numbers from 1 to _n_, with _n_ being the number of elements processed.

- **Time (ms):**  
  The execution time in milliseconds for each method. Lower times indicate higher performance efficiency. The variations in timing highlight the impact of synchronization overhead and parallel computation strategies.

### Thread Scaling Analysis
This section analyzes how performance scales with different thread counts:

- **Threads:** Number of threads used for computation
- **Atomic Sum (ms):** Time for atomic-based parallel summation
- **Reduce Sum (ms):** Time for reduction-based parallel summation  
- **ThreadPool Sum (ms):** Time for thread pool-based summation
- **Thread Overhead (ms):** Average overhead from thread creation and joining

**Key Observations:**
- **Optimal Thread Count:** Performance typically peaks at 4 threads, then may degrade due to context switching overhead
- **Reduce Sum Degradation:** Shows poor scaling due to false sharing and cache contention
- **ThreadPool Efficiency:** Generally maintains good performance across thread counts due to eliminated creation overhead
- **Thread Overhead:** Increases with more threads, showing the cost of thread management

### Workload Scaling Analysis
This section compares thread pool vs. regular threads across different data sizes:

- **Data Size:** Number of elements in the array
- **Threads (ms):** Time using regular thread creation/destruction
- **ThreadPool (ms):** Time using pre-created thread pool
- **Async (ms):** Time using `std::async` for asynchronous summation
- **Speedup T/TP:** Performance ratio (Threads time / ThreadPool time)
- **Speedup T/Async:** Performance ratio (Threads time / Async time)

**Key Observations:**
- **Small Workloads (1M-10M):** ThreadPool shows advantage due to reduced thread overhead
- **Medium Workloads (50M):** ThreadPool maintains good performance with 16% speedup
- **Large Workloads (100M):** Regular threads may perform slightly better due to better CPU utilization patterns
- **Speedup Variance:** Ratio varies based on workload characteristics and thread management efficiency

## Discussion: Pros and Cons of Each Method

### 1. Atomic Sum
**Pros:**
- Simple to implement for parallel updates.
- No need for explicit locks.
- Works well for small numbers of threads.

**Cons:**
- Contention on the atomic variable can limit scalability.
- Performance drops as thread count increases due to cache coherence traffic.
- Memory order selection affects both correctness and speed.

### 2. Reduce Sum
**Pros:**
- Each thread works independently, minimizing contention.
- No atomic operations or locks during computation.
- Good for NUMA systems and cache locality.

**Cons:**
- Requires extra memory for partial sums.
- Final aggregation step is sequential.
- False sharing may occur if partial sums are not properly padded.

### 3. ThreadPool Sum
**Pros:**
- Eliminates thread creation/destruction overhead for repeated tasks.
- Good resource management and scalability for many tasks.
- Useful for server-like or batch workloads.

**Cons:**
- More complex implementation (thread pool management).
- Overhead of task queue and synchronization.
- Not always optimal for one-off tasks.

### 4. Single-Threaded Sum
**Pros:**
- Easiest to implement and debug.
- No synchronization or parallelism overhead.
- Useful as a baseline for performance comparison.

**Cons:**
- No speedup from multiple cores.
- Slowest for large datasets on multicore systems.

### 5. Async Sum (std::async)
**Pros:**
- Very simple parallelism for divide-and-conquer algorithms.
- Automatic thread management (handled by the standard library).
- Futures make result collection and exception handling easy.
- No need to manually join threads.

**Cons:**
- Less control over thread pool and scheduling.
- Can create too many threads if not tuned (risk of oversubscription).
- Overhead from future/promise mechanism.
- Performance may be unpredictable depending on implementation.

## How to Compile and Run

### 1. Clone the Repository
Clone the repository from GitHub using the following commands:

```bash
git clone https://github.com/username/Memory-Orders.git
cd Memory-Orders
```

Alternatively, if you are working from a repository that provides runtime parameters:

```bash
git clone https://github.com/LyudmilaKostanyan/Parallel-Summation.git
cd Parallel-Summation
```

### 2. Build the Project
Use CMake to configure and build the project. Ensure that you have CMake and a C++ compiler (e.g., g++) installed:

```bash
cmake -S . -B build
cmake --build build
```

### 3. Run the Program

#### For Windows Users
For Windows, the executable is named `main.exe`. You can run it with an optional command-line parameter to specify the number of elements:
  
Example with arguments:
```bash
./build/main.exe --n 5000000
```
Example without arguments (default dataset size is used, e.g., 100,000,000 elements):
```bash
./build/main.exe
```

#### For Linux/macOS Users
For Linux/macOS, the executable is named `main`. Run it as follows:
  
Example with arguments:
```bash
./build/main --n 5000000
```
Example without arguments:
```bash
./build/main
```

## Parameters

- **--n:**  
  This command-line parameter allows you to specify the number of elements in the dataset. If this parameter is not provided, the program defaults to processing 100,000,000 elements. Adjust this parameter to test the performance of the summation methods with different data sizes.
