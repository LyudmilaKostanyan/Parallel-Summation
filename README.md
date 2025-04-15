# Memory Orders Benchmark

## Overview
This project is a C++ benchmarking tool designed to evaluate and compare different approaches to summing a large sequence of integers. It leverages multithreading to perform fast computations while testing how different memory ordering constraints impact performance and correctness in atomic operations.

## Problem Description
Summing a sequence of numbers efficiently is a common operation in many applications. This project implements three distinct methods to compute the sum:

- **Atomic Sum:**  
  Uses atomic operations with two different memory orders (`relaxed` and `seq_cst`) to safely update a shared total across multiple threads.

- **Reduce Sum:**  
  Distributes the data among multiple threads where each thread computes its own partial sum. The partial sums are later aggregated into a final result.

- **Single-Threaded Sum:**  
  Computes the sum sequentially using a single thread, serving as a baseline to compare the performance gains of multithreading.

The goal is to analyze the trade-offs between these approaches, particularly focusing on the performance impacts caused by different atomic memory orderings and the overhead of synchronization.

## Explanation of Some Topics

### Memory Ordering
- **`std::memory_order_relaxed`:**  
  This mode imposes minimal ordering constraints, providing faster updates but with minimal synchronization guarantees.
  
- **`std::memory_order_seq_cst`:**  
  Provides a strict global ordering of operations across threads, ensuring a consistent view at the cost of some performance overhead.

### Multithreading and Work Distribution
The program uses the C++ Standard Library's threading facilities (`std::thread`) to divide a large dataset into chunks. Each thread computes a partial sum for its assigned section. In the Atomic Sum approach, these partial sums are added to a shared atomic variable. In the Reduce Sum approach, the final summation is performed by aggregating locally computed partial sums.

### Performance Measurement
A timing utility from the `zen::timer` library is employed to record the execution time for each summing method. This measured time (in milliseconds) allows users to compare how different techniques perform under the same conditions and dataset sizes.

## Example Output
Below is an example of the output produced by the application:

```
Method              Memory Order        Sum                 Time (ms)           
--------------------------------------------------------------------------------
Atomic Sum          relaxed             5000000050000000    134.20              
Atomic Sum          seq_cst             5000000050000000    148.99              
Reduce Sum          N/A                 5000000050000000    969.43              
Single-Threaded     N/A                 5000000050000000    720.12    
```

## Explanation of Output
- **Method:**  
  Specifies which summation approach was used:
  - **Atomic Sum:** Uses atomic operations with the specified memory order.
  - **Reduce Sum:** Computes local partial sums in each thread, then aggregates them.
  - **Single-Threaded:** Computes the sum without any parallelization.

- **Memory Order:**  
  Indicates the memory ordering used for atomic operations. For non-atomic methods, it displays `N/A`.

- **Sum:**  
  The computed total sum, which is the result of summing numbers from 1 to _n_. The shown sum (5000000050000000) corresponds to the formula *n(n + 1)/2* when _n_ is 100,000,000 (the default dataset size).

- **Time (ms):**  
  The execution time in milliseconds for each summation method. Lower times indicate faster performance. The results highlight that while atomic operations can be efficient with relaxed ordering, reducing the workload using local partial sums may incur additional overhead in the aggregation phase.

## How to Compile and Run

### 1. Clone the Repository
Clone the repository from GitHub with the following commands:

```bash
git clone https://github.com/username/Parallel-Summation.git
cd Parallel-Summation
```

Alternatively, if you are working with a repository that accepts runtime parameters:

```bash
git clone https://github.com/LyudmilaKostanyan/Instruction-Reordering-Visualization.git
cd Instruction-Reordering-Visualization
```

### 2. Build the Project
Use CMake to configure and build the project. Ensure you have CMake and a C++ compiler (e.g., g++) installed:

```bash
cmake -S . -B build
cmake --build build
```

### 3. Run the Program

#### For Windows Users
If you want to run the program with a custom dataset size, use the `--n` parameter. For example:

Example with arguments:
```bash
./build/main.exe --n 5000000
```
Example without arguments (uses the default dataset size of 100,000,000 elements):
```bash
./build/main.exe
```

#### For Linux/macOS Users
For Linux or macOS, the executable is named `main`. Run it as follows:

Example with arguments:
```bash
./build/main --n 5000000
```
Or without arguments:
```bash
./build/main
```

## Parameters

- **--n:**  
  This command-line parameter allows you to specify the number of elements in the dataset. When omitted, the program defaults to processing 100,000,000 elements. Adjust this parameter based on the desired workload for performance testing.
