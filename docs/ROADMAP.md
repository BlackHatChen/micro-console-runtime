# Micro Console Runtime - Engineering Roadmap
The project simulates a constrained embedded runtime environment.
This document outlines the development milestones for the project.

## v0.1.0: Core Infrastructure & Allocator
Foundation of the runtime environment with a deterministic memory model.
- [x] **Build System & Environment**
  - Project structure design (src, include, tests, docs).
  - CMake-based build system with strictly enforced compiler flags (`-Wall -Wextra -Werror`).
  - Dockerized development container for reproducible builds.
- [x] **Quality Control Hooks**
  - Git Hooks infrastructure and commit template for enforcing Conventional Commits.
- [x] **Slab Allocator Implementation**
  - Fixed-size block partitioning strategies.
  - O(1) allocation/deallocation time complexity (Using embedded free lists).
  - Memory alignment (Aligned to a word size - 8 bytes in 64-bit systems).

## v0.2.0: Reliability, Testing and CI
Enhancing system stability through automated testing and continuous integration.
- [x] **Automated Testing Framework**
  - Integration of **Google Test** by CMake FetchContent.
  - Unit tests coverage for `SlabAllocator` (Boundary check, edge case, OOM handling).
- [x] **CI/CD Pipeline Automation**
  - **GitHub Actions workflow** for automated building and testing on Linux/Ubuntu runners.
- [x] **Performance Profiling**
  - Integration of **Google Benchmark**.
  - Throughput analysis: `SlabAllocator` vs. System `malloc`.

## v0.3.0: Hardware Optimization & Architecture
Low-level optimizations targeting specific hardware constraints and modern CPU architectures.
- [x] **Arbitrary Alignment Engine**
  - Enforce custom alignment boundaries at the Pool initialization stage.
  - Implement cross-platform OS-level aligned memory APIs (`_aligned_malloc` for Windows, `posix_memalign` for POSIX).
- [x] **Cache Coherency & SIMD Compatibility**
  - Enforce 64-byte (Cache Line) alignment to prevent False Sharing in multi-core scenarios.
  - Validate memory alignment constraints for SIMD instructions (e.g., 16-byte for NEON, 32-byte for AVX).
- [ ] **Segregated Free Lists (Slab Classes)**
  - Implement a `SlabManager` dispatcher to manage multiple generic size classes (e.g., 16B, 32B, 64B, 128B).
  - O(1) size class routing to provide a generalized, fragmentation-free memory allocation interface.

## v0.4.0: Memory Safety & Debugging Tools
Advanced diagnostics for detecting runtime memory corruption.
- [ ] **Corruption Detection**
  - **Redzone Protection**: Buffer overflow detection using specific patterns (0xDEADBEEF).
  - **Poisoning**: Use-After-Free detection using specific patterns (0xCCCCCCCC).
- [ ] **Fragmentation Analysis**
  - Memory heatmap generation for visual fragmentation debugging.

## v1.0.0: System Integration & Delivery
Full runtime integration with simulation subsystems.
- [ ] **Subsystem Integration**
- [ ] **Cross-Platform Verification**
- [ ] **Documentation**: API Reference generation using Doxygen.