# Micro Console Runtime - Engineering Roadmap
The project simulates a constrained embedded runtime environment.
This document outlines the development milestones for the project.

## v0.1.0: Core Infrastructure & Allocator
Foundation of the runtime environment with a deterministic memory model.
- [x] **Build System & Environment**
  - Project structure design (src, include, tests, docs).
  - CMake-based build system with enforced compiler flags (`-Wall -Wextra -Werror`).
  - Dockerized development container for reproducible builds.
- [x] **Quality Control Hooks**
  - Git Hooks infrastructure and commit template for enforcing Conventional Commits.
- [x] **Slab Allocator Implementation**
  - Fixed-size block partitioning strategies.
  - O(1) allocation/free time complexity (Using embedded free lists).
  - Memory alignment (Aligned to a word size).

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
- [x] **Segregated Free Lists (Slab Classes)**
  - Implement a `SlabManager` dispatcher to manage multiple generic size classes (e.g., 16B, 32B, 64B, 128B).
  - O(1) size class routing to provide a generalized, fragmentation-free memory allocation interface.

## v0.4.0: Memory Safety & Corruption Detection
Core fail-fast diagnostics for detecting runtime memory corruption in a constrained environment.
- [ ] **Compile-Time Safety Toggles**
  - Implement `#ifdef MCR_DEBUG_MEMORY` toggles to ensure absolute zero-overhead in Release builds.
- [ ] **Corruption Detection (Sanitizers)**
  - **Redzone Protection (Canaries)**: Insert `0xDEADBEEF` magic patterns before/after allocated blocks to catch out-of-bounds (OOB) writes.
  - **Poisoning**: Fill freed memory blocks with `0xCCCCCCCC` to ensure Use-After-Free (UAF) anomalies trigger immediate, predictable crashes.
  - **Endian-Safe Implementation**: Ensure magic numbers are reliably verifiable across different CPU architectures (x86/ARM).
- [ ] **Automated Memory Validation (CI)**
  - Implement **GoogleTest Death Tests** to validate fail-fast mechanisms upon memory corruption.
  - Add separated **GitHub Actions pipelines**: Debug (Sanitizers enabled) for safety validation, and Release for throughput benchmarking.

## v0.5.0: Memory Diagnostics & Advanced Profiling
Advanced state tracking, recovery mechanisms, and cross-platform verification.
- [ ] **Lifecycle & Leak Diagnostics**
  - **Active Allocation Registry**: Track allocated block lifecycles to detect memory leaks.
  - **Allocation Site Tracking**: Capture `__FILE__` and `__LINE__` via macros during allocation to pinpoint the exact source of leaks or redzone violations.
  - **Fragmentation Analysis**: Generate **Memory Heatmaps** based on registry data for visual fragmentation debugging.
- [ ] **Advanced Safety & Recovery**
  - **Custom OOM Handler**: Implement an Out-of-Memory callback mechanism, allowing the application layer to flush caches or perform emergency saves before aborting.
  - **Thread Ownership Assertion**: Record `std::this_thread::get_id()` upon allocation to detect illegal cross-thread deallocations in lock-free contexts.
- [ ] **Cross-Platform CI Verification**
  - **ARM Architecture Verification**: Utilize QEMU within CI to verify alignment constraints and endian-safe redzones on ARM toolchains.

## v1.0.0: System Integration & Delivery
Full runtime integration with simulation subsystems.
- [ ] **Subsystem Integration**
- [ ] **Cross-Platform Verification**
- [ ] **Documentation**: API Reference generation using Doxygen.