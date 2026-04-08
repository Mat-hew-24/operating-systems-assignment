# Multithreaded Perfect Number Checker

**Author:** Abhinav Mathew (Roll No: B240115CS, Batch: CS01)  
**Environment:** POSIX / Linux  

## 1. Project Overview

This project implements a highly optimized, multithreaded command-line application in C to determine whether a given integer **N** is a perfect number. It leverages the POSIX threads (`pthreads`) library to distribute the computationally expensive task of finding and summing proper divisors across multiple CPU cores.

A **perfect number** is a positive integer that is strictly equal to the sum of its proper divisors (excluding itself). 
* Example 1: `6 = 1 + 2 + 3`
* Example 2: `28 = 1 + 2 + 4 + 7 + 14`

By combining mathematical optimizations with low-level thread synchronization, this program handles massive integers (up to the 64-bit `long long` limit) exponentially faster than a standard single-threaded brute-force approach.

---

## 2. Mathematical Optimizations

A naive approach iterates from `1` to `N-1`, yielding an $O(N)$ time complexity. For a large perfect number like 8,589,869,056, this would take an unreasonable amount of time. This implementation introduces two critical optimizations:

1.  **Square Root Bound:** The search space is drastically reduced from `[1, N)` to `[1, floor(sqrt(N))]`.
2.  **Divisor Pairing:** For every valid divisor `i` found in the reduced range, its corresponding pair `N/i` is also instantly added to the sum.

**Guard Clauses to Prevent Data Corruption:**
* **Perfect Square Guard:** If `N` is a perfect square (e.g., 25), when `i = 5`, `N/i` is also 5. The program explicitly checks `if (pair != i)` to prevent double-counting the square root.
* **Proper Divisor Guard:** When `i = 1`, the pair `N/1 = N`. Since perfect numbers only sum *proper* divisors, the program checks `if (pair != N)` to ensure the number itself is excluded from the accumulator.

---

## 3. System Architecture & Concurrency Model

### Workload Partitioning
The program avoids race conditions and uneven execution times by utilizing a **remainder-aware partitioning algorithm**. 

The total mathematical range `[1, floor(sqrt(N))]` is divided by the requested number of threads `P`. 
* `quotient = limit / P`
* `remainder = limit % P`

To ensure exact coverage without gaps or overlaps, the first `remainder` threads are assigned one extra calculation. Each thread is passed a heap-allocated `interval` struct defining its specific `start` and `end` bounds.

### Synchronization Strategy
Thread safety is maintained using a single `pthread_mutex_t`. 
To maximize parallel efficiency, threads **do not** lock the mutex during the search loop. Instead, each worker thread accumulates a `local_sum` within its own stack frame. The mutex is only acquired exactly once per thread at the very end of its execution to safely add its `local_sum` to the global `sum`.

This keeps mutex contention at strictly $O(1)$ per thread, preventing the parallelization overhead from slowing down the program.

---

## 4. Complexity Analysis

| Metric | Complexity | Explanation |
| :--- | :--- | :--- |
| **Time (Sequential)** | O(√N) | Base mathematical search limit. |
| **Time (Parallel)** | O(√N / P) | The search limit evenly distributed across P concurrent threads. |
| **Space** | O(P) | Memory required for P `pthread_t` handles and P heap-allocated `interval` structures. |
| **Mutex Acquisitions** | O(P) | Exactly one lock and unlock operation per thread. |
| **Critical Section** | O(1) | A single addition operation per thread. |

*Note: Optimal performance is achieved when `P` matches the physical core count of the host CPU. Setting `P` significantly higher than hardware limits will introduce context-switching overhead.*

---

## 5. Build & Execution

### Prerequisites
* GCC (GNU Compiler Collection)
* A POSIX-compliant operating system (Linux/macOS)
* Make sure to link the Math and Pthread libraries.

### Compilation
```bash
gcc B240115CS_A2.c -o a.out -lm -lpthread
