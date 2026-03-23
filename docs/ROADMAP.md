# Micro Console Runtime - Engineering Roadmap
- This roadmap lists version goals and acceptance for v0.x and the final integration at v1.0.0.
- **Scope (v0.x):** memory subsystem (allocator + tests/benchmark/CI/tooling); other subsystems are planned (not delivered yet).
- See [README → Current Status](../README.md#current-status). Commands live in [README → Quick Start](../README.md#quick-start).

## v0.1.0: Core Infrastructure & Allocator
**Goal:** Make the build reproducible and provide a minimal slab allocator.

**Why/Context:** Keep the base stable so later changes remain traceable.
- [x] **Build System & Environment**
  - Project structure (`src`, `include`, `tests`, `docs`).
  - Modern CMake with strict warnings.
  - Docker image for reproducible builds.
- [x] **Quality Control Hooks**
  - Git hooks and a commit message template.
- [x] **Slab Allocator**
  - Fixed-size blocks with embedded free lists (O(1) `Allocate`/`Free`).
  - **Historical alignment:** `sizeof(void*)`.

**Acceptance**
- Configure and build via CMake on the reference host (see [README → Quick Start](../README.md#quick-start)).
- Allocator public API is present (O(1) `Allocate`/`Free`).
- Commit template/hooks are available from v0.1.0.

**References**
- [README → Quick Start](../README.md#quick-start)
- [Dockerfile](../Dockerfile)
- [Header](../include/slab_allocator.h)
- [Source](../src/slab_allocator.cpp)

## v0.2.0: Reliability, Tests, and CI
**Goal:** Introduce automated tests, CI, and a benchmark method.

**Why/Context:** Make changes verifiable and prevent regressions.
- [x] **Unit Tests (GoogleTest)**
  - Cover the allocator's basic contract (`Allocate`/`Free`) success, OOM, and boundary cases.
- [x] **CI Pipeline**
  - GitHub Actions workflow builds and runs tests on a Linux runner.
- [x] **Benchmark (Google Benchmark)**
  - Release-mode benchmark that compares the slab allocator vs `malloc` on a default small-object workload.

**Acceptance**
- CI green on a clean Linux runner: configure, build, and run tests using the commands in [README → Quick Start](../README.md#quick-start).
- Unit tests cover: success path, OOM behavior, and boundary conditions.
- A release benchmark (Google Benchmark) runs and reports results; on the same machine and the default workload, slab is faster than `malloc`.

**References**
- [README → Quick Start](../README.md#quick-start)
- [CI](../.github/workflows/ci.yml)
- [Tests](../tests/test_slab.cpp)
- [Benchmark](../tests/benchmark_slab.cpp)

## v0.3.0: Hardware Optimization & Architecture
**Goal:** Introduce multi-size-class routing without linear scans.

**Why/Context:** Extend the v0.1.x single-size slab into multiple size classes.
- [x] **Arbitrary Alignment Engine**
  - Satisfy user-specified alignment with a platform floor (effective alignment = `max(alignment_, sizeof(void*))`).
  - Cross-platform aligned allocation APIs (`_aligned_malloc` for Windows, `posix_memalign` for POSIX).
- [x] **Cache Coherence & SIMD Compatibility**
  - Enable cache-line (64-byte) alignment for designated classes/paths to reduce the risk of false sharing.
  - Validate class-specific alignment (e.g., NEON 16-byte, AVX 32-byte).
- [x] **Segregated Free Lists (Slab Classes)**
  - `SlabManager` manages multiple size classes (16, 32, 64, 128, 256, 512, 1024 bytes).
  - O(1) size-class routing (no linear scans); fragmentation controlled per class.

**Acceptance**
- Targeted alignment tests pass for representative classes: SIMD (32-byte) and cache-line (64-byte).
- Effective alignment policy is documented: requests with `alignment_ ≥ class_min` are satisfied; the platform floor is `sizeof(void*)`.
- Routing is validated as O(1) by design notes: bit-scan-based mapping (`_BitScanReverse` / `__builtin_clzll`), **no linear scans**.

**References**
- [Allocator Header](../include/slab_allocator.h)
- [Allocator Source](../src/slab_allocator.cpp)
- [Manager Header](../include/slab_manager.h)
- [Manager Source](../src/slab_manager.cpp)
- [Allocator Test](../tests/test_slab.cpp)
- [Manager Test](../tests/test_manager.cpp)

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