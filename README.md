# Micro-Console Runtime Environment
[![C++ CI Pipeline](https://github.com/BlackHatChen/micro-console-runtime/actions/workflows/ci.yml/badge.svg)](https://github.com/BlackHatChen/micro-console-runtime/actions/workflows/ci.yml)

> A high-performance, deterministic C++ runtime simulation for constrained embedded systems.

## Current Status
**v0.3.1** (Stabilization on `fix/v0.3-stabilize`). Next: **v0.4.0** (Memory Safety & Debugging Tools).

## Overview
This project implements a lightweight runtime environment that simulates the strict resource constraints of embedded systems. It focuses on deterministic memory management, cache-line alignment/coherency, and system-level reliability with industrial delivery (CMake, CI, Docker, tests, benchmarks).

## Key Features
- **Deterministic Memory Model**: `SlabAllocator` + `SlabManager` (Segregated free lists), O(1) `Allocate`/`Free`, external fragmentation controlled. (*Throughput*: Slab is **≥1.5×** faster than `malloc` on the default small-object workload.)
- **Hardware-Aware Design**: O(1) size-class routing by bit-scan instructions (`__builtin_clzll` / `_BitScanReverse`), arbitrary memory alignment support (e.g., NEON 16-byte, AVX 32-byte, cache-line 64-byte).
- **Zero Per-allocation Metadata**: Deallocation uses explicit `(size, alignment)` to keep O(1) free and maximize usable memory density.
- **Reliability & Safety**: C++17/RAII, cross-platform aligned APIs (`_aligned_malloc` / `posix_memalign`).
- **Reproducibility**: Dockerized environment + GitHub Actions.

## Quick Start
```bash
# 1. Clone the Repo.
git clone https://github.com/BlackHatChen/micro-console-runtime.git
cd micro-console-runtime

# (Optional) Developer setup: enable Git hooks; auto-set commit template if .gitmessage exists.
bash scripts/setup-dev.sh
# Verify (expected: .githooks; .gitmessage if present)
git config --local --get core.hooksPath
git config --local --get commit.template || true

# (Optional) Run in Docker.
docker build -t micro-runtime-env . # Build the dev image.
# Enter the Docker environment (Windows PowerShell ${PWD} vs POSIX $(pwd)).
docker run --rm -it -v "$(pwd)":/app micro-runtime-env 

# 2. Build (Debug by default.)
cmake -S . -B build
cmake --build build -j

# 3. Run unit tests.
ctest --test-dir build --output-on-failure

# 4. Run benchmarks (Use Release for build and throughput).
cmake -S . -B build-rel -DCMAKE_BUILD_TYPE=Release
cmake --build build-rel -j
./build-rel/bin/mcr_benchmark
```

## Roadmap
We follow structured milestones towards v1.0.0. See [docs/ROADMAP.md](docs/ROADMAP.md).

## License
This project is licensed under the [MIT License](LICENSE).