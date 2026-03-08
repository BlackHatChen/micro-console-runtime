# Micro-Console Runtime Environment

[![C++ CI Pipeline](https://github.com/BlackHatChen/micro-console-runtime/actions/workflows/ci.yml/badge.svg)](https://github.com/BlackHatChen/micro-console-runtime/actions/workflows/ci.yml)

> A high-performance, deterministic C++ runtime simulation for constrained embedded systems.

## 📖 Overview
This project implements a lightweight runtime environment designed to simulate the strict resource constraints of modern embedded devices. It focuses on deterministic memory management, cache coherency, and system-level reliability, mirroring the architecture of embedded systems and RTOS (Real-Time Operating Systems).

**Current Status**: `v0.3.0` completed. In progress: `v0.4.0` (Memory Safety & Debugging Tools).

## ✨ Key Features
* **Deterministic Memory Model**: Custom `SlabAllocator` and `SlabManager` (Segregated Free Lists) eliminate external fragmentation and ensure predictable allocation in O(1) time.
  * *Performance*: Achieve **~4.1x throughput improvement** (4.03x speedup) over standard `std::malloc` in high-frequency small object allocation benchmarks (tested by Google Benchmark).
* **Hardware-Aware & Branchless Design**: 
  * Implement O(1) size class routing using hardware bit-scan instructions (`__builtin_clzll` / `_BitScanReverse`).
  * Enforce arbitrary memory alignment (e.g., AVX 32-byte, Cache-Line 64-byte) for SIMD operations and RISC architectures.
* **Zero Metadata Overhead**: Use C++14 Sized Deallocation semantics (`operator delete(void*, std::size_t)`) to completely eliminate header padding, maximizing memory density.
* **Reliability & Safety**: Build with RAII (`std::make_unique`, `std::array`) and modern C++17 standards to prevent memory leaks. Cross-platform OS API integration (`_aligned_malloc` / `posix_memalign`) ensures safe aligned memory boundaries.
* **Reproducibility**: Fully Dockerized development environment for consistent cross-platform builds and CI/CD integration via GitHub Actions.

## 📊 Performance Benchmark

Benchmarks were conducted using [Google Benchmark](https://github.com/google/benchmark) to compare the `SlabAllocator` against the OS standard `std::malloc`.

**Test Environment:**
* Compiler optimization: Release mode (`-O3`)
* Scenario: High-frequency batch allocation and deallocation of 24-byte objects.

**Results:**

| Benchmark | Time | CPU | Iterations |
| :--- | :--- | :--- | :--- |
| `BM_SystemMalloc` | 8171 ns | 7954 ns | 86,163 |
| `BM_SlabAllocator` | 2025 ns | 1971 ns | 353,274 |

*Analysis: The custom `SlabAllocator` achieves a **~4.03x speedup** in CPU time and processes roughly **4.1x more iterations** within the same time window compared to a general-purpose allocator. This proves the extreme efficiency of its O(1) embedded free list design and branchless routing in constrained scenarios.*

## 🚀 Quick Start (Recommended)

The easiest way to build and run the project is using the provided Docker environment.

### Prerequisites
* [Docker](https://www.docker.com/) installed on your machine.

### Build & Run
```
# 1. Clone the repository
git clone https://github.com/BlackHatChen/micro-console-runtime.git
cd micro-console-runtime

# 1.5 Developer Setup (Recommended)
# Enable Git hooks for consistent commit message enforcement across machines.

# Option A: one-shot setup script
bash scripts/setup-dev.sh

# Option B: manual setup
git config core.hooksPath .githooks
chmod +x .githooks/commit-msg

# 2. Build the Docker environment
docker build -t micro-runtime-env .

# 3. Enter the development container
docker run --rm -it -v $(pwd):/app micro-runtime-env

# --- Inside the Container ---

# 4. Build the project
mkdir -p build && cd build

# You can build the Makefile with Debug Mode.
cmake ..

# Or with -O3 optimization (Release Mode).
cmake -DCMAKE_BUILD_TYPE=Release ..

# You can compile the codes with default settings.
make

# Or compile in parallel with multi-threads.
make -j$(nproc)

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