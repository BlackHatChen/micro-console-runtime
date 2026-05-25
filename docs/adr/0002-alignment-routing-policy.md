# Include alignment in size-class routing decisions

## Status
Accepted

## Context
- Requests in this project carry both size and alignment requirements.
- If class selection does not reflect both size and alignment, the routing rule becomes incomplete.
- The current allocator subsystem avoids per-allocation metadata, so deallocation relies on caller-supplied request metadata to route a block back to its size class.

## Decision
- Size-class routing considers not only request size but also alignment.
- In the current policy, the routing key is `max(request_size, alignment)`.
- A suitable class is the smallest managed size class whose class size is greater than or equal to the routing key.
- Allocation and deallocation use the same routing inputs; callers must provide the same `(request_size, alignment)` pair to `Free()`.

## Consequences

### Pros
- Requests are routed to size classes that satisfy both the requested size and alignment.
- Treating alignment as a routing input makes class selection a single explicit rule.
- Defining the routing key makes the expected target class easier to document, test, and keep consistent with implementation behavior.
- Using the same routing inputs for allocation and deallocation preserves routing symmetry without storing per-allocation metadata.

### Trade-offs
- The routing rule is more complex than routing based only on request size.
- Callers must use the same `(request size, alignment)` pair symmetrically for allocation and deallocation.
- Passing a mismatched pair can route a block back to the wrong size class and is a contract violation.
- Without per-allocation metadata or side tracking, the manager cannot reliably verify pointer ownership and allocation-site symmetry.
- Full deallocation validation is left to future debug-oriented diagnostics rather than the current zero-metadata route-back path.

## Alternatives
- Route only by request size and handle alignment separately in the allocation path.
  - Not adopted, because this would decouple class selection from alignment requirements and make routing behavior harder to describe and test consistently.
- Expose class selection to the caller and let size-class routing ignore alignment.
  - Not adopted, because this would push class-selection rules out to the caller, increase usage burden, and make the routing boundary less stable.
- Store per-allocation metadata so `Free()` can recover the original size class from the pointer alone.
  - Not adopted, because it would add per-allocation overhead and change the current zero-metadata contract.