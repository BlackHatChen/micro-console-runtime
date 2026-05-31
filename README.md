# Micro-Console Runtime
[![C++ CI Pipeline](https://github.com/BlackHatChen/micro-console-runtime/actions/workflows/ci.yml/badge.svg)](https://github.com/BlackHatChen/micro-console-runtime/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)  
Micro-Console Runtime is a C++ system-oriented project for exploring selected runtime support components under constrained resources. The current v0.x scope focuses on an allocator-centered memory subsystem.

> Note: In the current allocator subsystem, "constrained resources" refers to bounded pool-managed memory and fixed-size allocation paths.

## Current Status
- Latest released version: **v0.3.2**

## Project Rationale

### Why this project
The project addresses and validates low-level system behavior in a constrained-resource environment. The name `Micro-Console Runtime` reflects a long-term vision of a small, bounded execution environment that can host and coordinate multiple runtime subsystems, while the current work focuses on the execution-time support layer rather than application logic.

### Why an allocator subsystem
Memory management is one of the low-level runtime responsibilities, and an allocator is a subsystem for defining allocation/deallocation behavior, alignment handling, routing rules, and their related contracts and tests. It also provides a focused and verifiable entry point for the current runtime-oriented work.

### Why a slab allocator
A slab allocator divides memory into fixed-size classes, which fits the current memory subsystem's fixed-size allocation paths and helps control external fragmentation in pool-managed memory. It also provides predictable allocation/deallocation costs and supports class-based routing and alignment-aware handling.

## Current Implementation Scope
The current implementation focuses on the allocator-centered memory subsystem:

### Core implementation
- `SlabAllocator`
- `SlabManager`

### Supporting validation and tooling
- unit tests
- CI pipeline
- initial benchmark work for allocator comparison
- Docker-based reproducible build environment

Other subsystems are planned separately and are not yet part of the delivered implementation.

## Key Features
- **Fixed-Size Allocator**: `SlabAllocator` provides O(1) allocation/deallocation from a fixed-size pool using an embedded free list.
- **O(1) Size-Class Routing**: `SlabManager` routes requests by `max(size, alignment)` using bit-scan-based size-class mapping and alignment-aware class selection without linear scans.
- **Explicit Deallocation Contract**: Multi-class deallocation requires caller-supplied `(size, alignment)` instead of per-allocation metadata, preserving O(1) routing symmetry across allocation and deallocation.
- **Validation and Reproducibility**: Public behavior is supported by unit tests, CI, and a reproducible build environment. Initial benchmark work is available for fixed-workload allocator comparison.

## Quick Start
```bash
# 1. Clone the Repo.
git clone https://github.com/BlackHatChen/micro-console-runtime.git
cd micro-console-runtime

# (Optional, Bash/Git Bash/WSL) Developer setup: enable Git hooks; auto-set commit template if .gitmessage exists.
bash scripts/setup-dev.sh
# Verify (expected: .githooks; .gitmessage if present)
git config --local --get core.hooksPath
git config --local --get commit.template || true

# (Optional) Run in Docker.
docker build -t micro-runtime-env . # Build the dev image.

# Enter the Docker environment.
# POSIX shells:
docker run --rm -it -v "$(pwd)":/app -w /app micro-runtime-env

# Windows PowerShell:
# docker run --rm -it -v "${PWD}:/app" -w /app micro-runtime-env

# 2. Build (Debug).
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel

# 3. Run unit tests.
ctest --test-dir build --output-on-failure

# 4. (Optional) Run the allocator benchmark (Release build).
cmake -S . -B build-rel -DCMAKE_BUILD_TYPE=Release
cmake --build build-rel --parallel
./build-rel/bin/mcr_benchmark
```

## Roadmap
The current v0.x roadmap is tracked in [ROADMAP](docs/ROADMAP.md).

## License
The project is licensed under the [MIT License](LICENSE).