# Micro Console Runtime - Engineering Roadmap
- This roadmap tracks version goals and acceptance across v0.x, with a later integration milestone reserved for broader runtime work.
- **Scope (v0.x):** memory subsystem work centered on the allocator, with supporting validation and tooling. Other subsystems are still planned.
- See [README → Current Status](../README.md#current-status). Related commands are listed in [README → Quick Start](../README.md#quick-start).

---

## v0.1.0: Core Infrastructure & Allocator
**Goal:** Establish a reproducible build setup and deliver a minimal slab allocator.

**Why/Context:** Provide a stable starting point for later versions.
- [x] **Build System & Environment**
  - Project structure (`src`, `include`, `tests`, `docs`).
  - CMake build setup with strict warnings enabled.
  - Docker image for reproducible builds.
- [x] **Repository Workflow Setup**
  - Git hooks and a commit message template.
- [x] **Slab Allocator**
  - Fixed-size blocks with embedded free lists.
  - **Historical alignment:** `sizeof(void*)`.

**Acceptance**
- Configure and build via CMake using the documented setup.
- The allocator's API is available and supports O(1) `Allocate`/`Free`.
- Git hooks and the commit message template are available in the repository.

**References**
- [README → Quick Start](../README.md#quick-start)
- [Dockerfile](../Dockerfile)
- [Header](../include/slab_allocator.h)
- [Source](../src/slab_allocator.cpp)

---

## v0.2.0: Reliability, Tests, and CI
**Goal:** Introduce automated tests, CI, and an initial allocator benchmark.

**Why/Context:** Make changes verifiable and reduce regression risk.
- [x] **Unit Tests (GoogleTest)**
  - Cover the allocator's core success, OOM, and boundary cases.
- [x] **CI Pipeline**
  - GitHub Actions workflow builds and runs tests on a Linux runner.
- [x] **Benchmark (Google Benchmark)**
  - Add an initial release-mode benchmark for allocator-vs-malloc comparison.

**Acceptance**
- Unit tests cover allocator success, OOM behavior, and boundary conditions.
- CI green on a clean Linux runner: configure, build, and run tests.
- An initial allocator benchmark is present and runnable for a fixed small-object batch workload.

**References**
- [README → Quick Start](../README.md#quick-start)
- [CI](../.github/workflows/ci.yml)
- [Tests](../tests/slab_allocator_test.cpp)
- [Benchmark](../tests/benchmark_slab.cpp)

---

## v0.3.0: Alignment Support & Routing Architecture
**Goal:** Introduce alignment-aware allocation and multi-size-class routing without linear scans.

**Why/Context:** Extend the single-size slab into multiple size classes.
- [x] **Arbitrary Alignment Support**
  - Satisfy user-specified alignment with a platform floor by `max(requested alignment, sizeof(void*))`.
  - Cross-platform aligned allocation APIs (`_aligned_malloc` for Windows, `posix_memalign` for POSIX).
- [x] **Representative Alignment Cases**
  - Support class-specific alignment outcomes, including 16-byte, 32-byte, and 64-byte requests.
  - Validate that aligned requests route to a size class that satisfies the requested alignment.
- [x] **Segregated Free Lists (Slab Classes)**
  - `SlabManager` manages multiple size classes (16, 32, 64, 128, 256, 512, 1024 bytes).
  - O(1) size-class routing across fixed-capacity segregated classes.

**Acceptance**
- Targeted alignment tests pass for representative aligned request cases.
- Effective alignment follows `max(requested alignment, sizeof(void*))`.
- Size-class routing is implemented without linear scans and is documented as O(1).

**References**
- [Allocator Header](../include/slab_allocator.h)
- [Allocator Source](../src/slab_allocator.cpp)
- [Allocator Test](../tests/slab_allocator_test.cpp)
- [Manager Header](../include/slab_manager.h)
- [Manager Source](../src/slab_manager.cpp)
- [Manager Test](../tests/slab_manager_test.cpp)

---

## v0.3.1: Stabilization & Validation
**Goal:** Stabilize allocator and manager behavior by aligning tests, public contracts, and code comments.

**Why/Context:** Keep the scope stable while making allocator and manager behavior clearer and more testable.
- Stabilization-first: bug fixes, contract-alignment changes, tests, CI, and comments are allowed.
- No new subsystem scope is introduced.
- Keep scope tight to avoid unnecessary changes and regressions.

**Acceptance**
- Routing symmetry for allocation and deallocation with the same `(size, alignment)` pair is covered by tests.
- Alignment validation is performed at the manager allocation entry.
- Allocator and manager code, tests, and public contract wording are aligned with current behavior.

---

## v0.4.0: Debug Memory Diagnostics & Corruption Detection
**Goal:** Introduce debug-oriented diagnostics for detecting memory corruption and invalid memory use.

**Why/Context:** Add fail-fast validation mechanisms that help detect memory corruption earlier.
- [ ] **Debug-Only Safety Toggles**
  - Enable memory diagnostics only in debug builds so release builds do not include these checks.
- [ ] **Corruption Detection**
  - Add diagnostics for out-of-bounds writes and invalid reuse of freed memory.
- [ ] **Validation Coverage**
  - Add automated validation for fail-fast diagnostics and ensure they can be run in debug/CI workflows.

**Acceptance**
- Debug-only memory diagnostics can be enabled without changing normal release behavior.
- Out-of-bounds and invalid reuse checks are present in debug-oriented paths.
- Diagnostic checks have automated validation coverage.

---

## v0.5.0: Memory Diagnostics & Observability
**Goal:** Extend allocator diagnostics with lifecycle visibility, failure diagnostics, and validation infrastructure.

**Why/Context:** Extend diagnostics work with better visibility into allocator state and broader validation support.
- [ ] **Allocation Lifecycle Diagnostics**
  - Track active allocations and surface leak-related state.
  - Include allocation-site information for leaks and invalid-use diagnostics.
  - Summarize fragmentation-related state for better allocator visibility.
- [ ] **Failure and Usage Diagnostics**
  - Provide diagnostics or integration points for allocator failure handling.
  - Add debug checks for invalid ownership or usage patterns where applicable.
- [ ] **Cross-Platform Validation**
  - Expand validation workflows to cover additional target environments where relevant.

**Acceptance**
- Allocation-lifecycle diagnostics provide visibility into active allocations and leak-related state.
- Additional diagnostic information is available for failure or invalid-usage scenarios.
- Validation workflows can cover these diagnostics in at least one additional target environment or configuration.
