# Parallel Summation Benchmark

## Overview
This C++ project demonstrates parallel summation techniques by partitioning an array among multiple threads. It compares different methods for aggregating the results—using atomic operations with various memory orderings, a reduction-based approach, and a thread pool implementation—to explore trade-offs in correctness, complexity, and speed. The project includes comprehensive performance analysis across different thread counts and workload sizes.

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

## Example Output
An example run of the program may produce output similar to the following:

```
Thread Count: 8

=== Basic Performance Comparison ===
Method              Memory Order        Sum                 Time (ms)
--------------------------------------------------------------------------------
Atomic Sum          relaxed             5000000050000000    115.86
Atomic Sum          seq_cst             5000000050000000    138.69
Reduce Sum          N/A                 5000000050000000    910.52
ThreadPool Sum      N/A                 5000000050000000    122.01
Single-Threaded     N/A                 5000000050000000    744.05

=== Thread Scaling Analysis ===
Threads   Atomic Sum (ms)     Reduce Sum (ms)     ThreadPool Sum (ms)   Thread Overhead (ms)
----------------------------------------------------------------------------------------------        
1         303.72              460.48              302.13                382.09
2         186.18              703.09              191.48                444.63
4         130.94              824.91              125.41                477.92
8         155.79              1411.83             142.36                783.80

=== Workload Scaling Analysis ===
Data Size      Threads (ms)   ThreadPool (ms)   Speedup Ratio
-----------------------------------------------------------------
1000000        7.76           6.28              1.24
5000000        12.70          12.63             1.01
10000000       23.55          23.89             0.99
50000000       73.90          63.48             1.16
100000000      121.81         130.37            0.93
```

## Explanation of Output

### Basic Performance Comparison
- **Method:**  
  The column indicates the approach used:
  - **Atomic Sum:** Aggregates results using atomic operations with the specified memory order.
  - **Reduce Sum:** Accumulates partial sums computed by multiple threads.
  - **ThreadPool Sum:** Uses a pre-created thread pool to process summation tasks.
  - **Single-Threaded:** Uses a sequential method, serving as the performance baseline.

- **Memory Order:**  
  Displays the memory ordering for atomic operations. For non-atomic methods such as Reduce Sum, ThreadPool Sum, and Single-Threaded Sum, this field is marked as `N/A`.

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
- **Speedup Ratio:** Performance ratio (Threads time / ThreadPool time)

**Key Observations:**
- **Small Workloads (1M-10M):** ThreadPool shows advantage due to reduced thread overhead
- **Medium Workloads (50M):** ThreadPool maintains good performance with 16% speedup
- **Large Workloads (100M):** Regular threads may perform slightly better due to better CPU utilization patterns
- **Speedup Variance:** Ratio varies based on workload characteristics and thread management efficiency

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
