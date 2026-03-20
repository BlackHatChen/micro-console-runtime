# ADR-0001: Allocator Contract (v0.1.0 baseline)

## Status
Accepted (v0.1.0)

## Context
The initial slab allocator was delivered before tests/CI/bench were introduced.
We need a documented baseline to anchor future changes.

## Decision
- Historical alignment baseline: payloads are aligned to `sizeof(void*)`.
- Public API/contract: allocate/free with O(1) intent (embedded free lists).
- No per-allocation metadata; free requires `(ptr, size, alignment)` (zero-metadata design).

## Contracts
- On success, `allocate(size)` returns non-null; on OOM, behavior follows policy (to be defined).
- `free(ptr, size, alignment)` expects the exact `(size, alignment)` used by the allocation path.

## Consequences
- Future versions may raise the alignment floor (e.g., `alignof(std::max_align_t)`) or honor
  user-specified `alignment_` with a floor; tests/docs must be updated accordingly.

## References
- README: Quick Start (build) and Reference Host
- Roadmap v0.1.0