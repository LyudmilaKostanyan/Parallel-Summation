# Parallel Summation Benchmark

## Overview
This C++ project demonstrates parallel summation techniques by partitioning an array among multiple threads. It compares different methods for aggregating the results—using atomic operations with various memory orderings and a reduction-based approach—to explore trade-offs in correctness, complexity, and speed.

## Problem Description
Summing large arrays efficiently is a common problem in parallel computing. The challenge lies in managing shared data access and synchronization across multiple threads. This project implements three methods for calculating the sum of a large integer array:

1. **Atomic Sum:**  
   Threads update a shared atomic variable using the `fetch_add` operation with different memory orderings:
   - **Relaxed:** Provides minimal synchronization, offering higher performance but less ordering guarantees.
   - **Sequentially Consistent (seq_cst):** Ensures a strict global order of operations, enhancing correctness at the cost of additional overhead.

2. **Reduce Sum:**  
   The array is partitioned among several threads. Each thread computes a partial sum independently, and these partial sums are then aggregated in the main thread. This approach avoids locks during computation but requires additional aggregation.

3. **Single-Threaded Sum:**  
   A baseline method that performs the summation sequentially without multithreading, offering a point of comparison for performance metrics.

## Example Output
An example run of the program may produce output similar to the following:

```
Iterations: 100000000

Method              Memory Order        Sum                 Time (ms)           
--------------------------------------------------------------------------------
Atomic Sum          relaxed             5000000050000000    134.20              
Atomic Sum          seq_cst             5000000050000000    148.99              
Reduce Sum          N/A                 5000000050000000    969.43              
Single-Threaded     N/A                 5000000050000000    720.12    
```

## Explanation of Output
- **Method:**  
  The column indicates the approach used:
  - **Atomic Sum:** Aggregates results using atomic operations with the specified memory order.
  - **Reduce Sum:** Accumulates partial sums computed by multiple threads.
  - **Single-Threaded:** Uses a sequential method, serving as the performance baseline.

- **Memory Order:**  
  Displays the memory ordering for atomic operations. For non-atomic methods such as Reduce Sum and Single-Threaded Sum, this field is marked as `N/A`.

- **Sum:**  
  The computed total from summing the integers. In this case, the sum `5000000050000000` represents the mathematical result of summing numbers from 1 to _n_, with _n_ being the number of elements processed.

- **Time (ms):**  
  The execution time in milliseconds for each method. Lower times indicate higher performance efficiency. The variations in timing highlight the impact of synchronization overhead and parallel computation strategies.

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
