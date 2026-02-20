# Micro-Console Runtime Environment

> A high-performance, deterministic C++ runtime simulation for constrained embedded systems.

## 📖 Overview
This project implements a lightweight runtime environment designed to simulate the strict resource constraints of modern embedded devices. It focuses on deterministic memory management, cache coherency, and system-level reliability, mirroring the architecture of embedded systems and RTOS (Real-Time Operating Systems).

**Current Status**: `v0.1.0` (Core Infrastructure & Allocator)

## ✨ Key Features
* **Deterministic Memory Model**: Custom `SlabAllocator` eliminates external fragmentation and ensures predictable allocation latency (O(1) time complexity), crucial for real-time constraints.
* **Hardware-Aware Design**: Enforces strict memory alignment suitable for RISC architectures (e.g., ARM Cortex-A/R series commonly used in automotive and consumer electronics).
* **Reliability**: Built with strict RAII compliance and modern C++17 standards to prevent memory leaks and undefined behaviors.
* **Reproducibility**: Fully Dockerized development environment for consistent cross-platform builds and CI/CD integration.

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

# 5. Run the Slab Allocator test
./bin/mcr_test

```

## 🗺️ Roadmap

We follow a structured milestone roadmap towards a v1.0.0 release.
Please refer to [docs/ROADMAP.md](docs/ROADMAP.md) for detailed development plans.

## 📄 License

This project is licensed under the [MIT License](LICENSE).