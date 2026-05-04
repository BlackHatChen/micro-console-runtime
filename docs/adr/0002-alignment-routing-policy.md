# Include alignment in size-class routing decisions

## Status
Accepted

## Context
- Requests in this project carry both size and alignment requirements.
- If class selection does not reflect both size and alignment, the routing rule becomes incomplete.
- Once alignment may be handled at different stages of the allocation path, the expected target class for the same request becomes harder to describe, document, and test consistently.

## Decision
- Size-class routing considers not only request size but also alignment, and routes each request to the smallest suitable size class.

## Consequences

### Pros
- Requests are routed to size classes that satisfy both the requested size and alignment.
- Treating alignment as a routing input makes class selection a single explicit rule.
- That makes the expected target class for a request easier to document, routing tests easier to define, and both easier to keep consistent with implementation behavior.

### Trade-offs
- The routing rule is more complex than routing based only on request size.
- Callers must use the same `(request size, alignment)` pair symmetrically for allocation and deallocation.

## Alternatives
- Route only by request size and handle alignment separately in the allocation path.
  - Not adopted, because this would decouple class selection from alignment requirements and make routing behavior harder to describe and test consistently.
- Expose class selection to the caller and let size-class routing ignore alignment.
  - Not adopted, because this would push class-selection rules out to the caller, increase usage burden, and make the routing boundary less stable.