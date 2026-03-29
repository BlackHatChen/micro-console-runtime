# Micro-Console Runtime
[![C++ CI Pipeline](https://github.com/BlackHatChen/micro-console-runtime/actions/workflows/ci.yml/badge.svg)](https://github.com/BlackHatChen/micro-console-runtime/actions/workflows/ci.yml)

> An embedded-inspired C++ runtime project with a current v0.x focuses on the memory subsystem.

## Current Status
- Latest released version: **v0.3.0**
- In development: **v0.3.1** on `fix/v0.3-stabilize`
- Next planned milestone: **v0.4.0** (Memory Safety & Debugging Tools)

## Overview
Micro-Console Runtime is a systems-oriented C++ project inspired by constrained embedded environments and RTOS-style design goals.

The current delivered scope focuses on the memory subsystem:
- predictable fixed-size allocation paths
- size-class-based O(1) routing
- alignment-aware allocation for hardware-sensitive paths
- engineering evidence through tests, benchmarks, CI, and Docker

Other runtime subsystems are planned separately and are not yet delivered in v0.x.

## Current Implementation Scope
The current delivered implementation focuses on the memory subsystem:
- `SlabAllocator`
- `SlabManager`
- unit tests
- benchmark
- CI pipeline
- Docker-based reproducible build environment

Planned runtime subsystems such as simulation, message transport, and higher-level integration are not yet part of the delivered implementation.

## Key Features
- **Fixed-Size Allocator Core**: `SlabAllocator` provides O(1) allocation/deallocation from a single fixed-size pool using an embedded free list.
- **O(1) Size-Class Routing**: `SlabManager` routes requests by `max(size, alignment)` using bit-scan-based size-class mapping without linear scans.
- **Explicit Deallocation Contract**: Multi-class deallocation requires caller-supplied `(size, alignment)` instead of per-allocation metadata, preserving O(1) routing symmetry across allocation and free.
- **Alignment-Aware Design**: Supports alignment-sensitive allocation paths for hardware-aware use cases such as SIMD and cache-line alignment.
- **Engineering Evidence**: Public contracts are reinforced by tests, CI, benchmarks, and a Docker-based reproducible development environment.

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