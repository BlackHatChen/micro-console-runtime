# Changelog
All notable changes to this project will be documented in this file.

---

## [0.3.1] - Unreleased
This release focuses on contract and correctness stabilization for the memory subsystem without expanding subsystem scope.

### Fixed
- Fix `SlabManager` size-class deallocation routing so `Allocate()`/`Free()` use the same `max(size, alignment)` policy.
- Validate `SlabManager::Allocate()` alignment input and reject zero or non-power-of-two values with `std::invalid_argument`.

### Changed
- Tighten `SlabManager::Free()` usage contract: callers must supply the same `alignment` used at allocation time to preserve O(1) symmetric routing without per-allocation metadata and avoid silent misuse for over-aligned paths.
- Clarify allocator and manager contracts in public headers, especially around:
  - effective alignment
  - routing symmetry
  - constructor failure behavior
  - invalid free / undefined behavior boundaries
- Reclassify and simplify selected allocator/manager tests to reduce dependence on internal layout assumptions.
- Derive several slab test pool sizes from allocator contract rules instead of fixed platform-specific assumptions.

### Tests
- Add regression coverage for `(size, alignment)` routing symmetry and alignment validation.
- Reclassify selected tests to reduce dependence on internal layout assumptions.

### CI/Build/Tooling
- Align CI and local build flow with the documented Quick Start path.
- Improve development environment consistency across build, Docker, and hook setup.

### Docs
- Refine supplementary allocator and manager documentation.
- Add a `v0.3.1` stabilization section to the roadmap.

---

## [0.3.0] - 2026-03-02

### Added
- Arbitrary alignment support for slab allocation.
- Cross-platform aligned OS allocation paths:
  - `_aligned_malloc` on Windows/MSVC
  - `posix_memalign` on POSIX
- Segregated free-list manager (`SlabManager`) for multiple size classes:
  16, 32, 64, 128, 256, 512, 1024 bytes.
- O(1) size-class routing using bit-scan mapping.

### Changed
- Generalize allocation routing from a single fixed-size allocator to multiple power-of-two classes.
- Strengthen hardware-aware alignment behavior for SIMD/cache-line use cases.

### Performance
- Preserve O(1) allocation/free behavior while extending to multi-class routing.

---

## [0.2.0] - 2026-02-22

### Added
- GoogleTest-based unit test coverage for allocator basics, boundaries, and OOM.
- GitHub Actions CI pipeline for build + test on Linux.
- Google Benchmark integration for allocator throughput measurement.

### Changed
- Improve project reproducibility by aligning CI flow with documented build/run commands.

### Performance
- Establish baseline throughput comparison against system `malloc`.

---

## [0.1.0] - 2026-02-18

### Added
- Initial slab allocator implementation for fixed-size O(1) allocation/free.
- Dockerized development environment.
- CMake-based project skeleton with strict compiler warnings.
- Git hook / commit message standardization setup.

### Changed
- Establish the initial deterministic memory-management baseline for the micro-console runtime environment.