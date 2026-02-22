# Micro-Console Runtime Environment

[![C++ CI Pipeline](https://github.com/BlackHatChen/micro-console-runtime/actions/workflows/ci.yml/badge.svg)](https://github.com/BlackHatChen/micro-console-runtime/actions/workflows/ci.yml)

> A high-performance, deterministic C++ runtime simulation for constrained embedded systems.

## 📖 Overview
This project implements a lightweight runtime environment designed to simulate the strict resource constraints of modern embedded devices. It focuses on deterministic memory management, cache coherency, and system-level reliability, mirroring the architecture of embedded systems and RTOS (Real-Time Operating Systems).

**Current Status**: `v0.2.0` completed. In progress: `v0.3.0` (Hardware Optimization & Architecture).

## ✨ Key Features
* **Deterministic Memory Model**: Custom `SlabAllocator` eliminates external fragmentation and ensures predictable allocation latency (O(1) time complexity).
  * *Performance*: Achieved **~3x throughput improvement** (2.96x speedup) over standard `std::malloc` in high-frequency small object allocation benchmarks (tested by Google Benchmark).
* **Hardware-Aware Design**: Enforces strict memory alignment suitable for RISC architectures (e.g., ARM Cortex-A/R series commonly used in automotive and consumer electronics).
* **Reliability**: Built with strict RAII compliance and modern C++17 standards to prevent memory leaks and undefined behaviors.
* **Reproducibility**: Fully Dockerized development environment for consistent cross-platform builds and CI/CD integration.

## 📊 Performance Benchmark

Benchmarks were conducted using [Google Benchmark](https://github.com/google/benchmark) to compare the `SlabAllocator` against the OS standard `std::malloc`.

**Test Environment:**
* Compiler optimization: Release mode (`-O3`)
* Scenario: High-frequency batch allocation and deallocation of 24-byte objects.

**Results:**

| Benchmark | Time | CPU | Iterations |
| :--- | :--- | :--- | :--- |
| `BM_SystemMalloc` | 7524 ns | 7542 ns | 92,716 |
| `BM_SlabAllocator` | 2540 ns | 2545 ns | 276,917 |

*Analysis: The custom `SlabAllocator` achieves a **~2.96x speedup** in CPU time and processes roughly **3x more iterations** within the same time window compared to a general-purpose allocator. This proves the extreme efficiency of its O(1) embedded free list design in constrained scenarios.*

## 🚀 Quick Start (Recommended)

The easiest way to build and run the project is using the provided Docker environment.

### Prerequisites
* [Docker](https://www.docker.com/) installed on your machine.

### Build & Run
```
# 1. Clone the repository
git clone https://github.com/BlackHatChen/micro-console-runtime.git
cd micro-console-runtime

# 2. Build the Docker environment
docker build -t micro-runtime-env .

# 3. Enter the development container
docker run --rm -it -v $(pwd):/app micro-runtime-env

# --- Inside the Container ---

# 4. Build the project
mkdir -p build && cd build
cmake ..
make

# 5. Run the tests

# You can run the detailed unit tests by the executable:
./bin/mcr_test

# Or run the automated test using CTest:
ctest --output-on-failure

```

## 🗺️ Roadmap

We follow a structured milestone roadmap towards a v1.0.0 release.
Please refer to [docs/ROADMAP.md](docs/ROADMAP.md) for detailed development plans.

## 📄 License

This project is licensed under the [MIT License](LICENSE).