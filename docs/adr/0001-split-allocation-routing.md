# Separate fixed-size allocation from size-class routing responsibilities

## Status
Accepted

## Context
- The project needs to handle both fixed-size memory allocation/deallocation and request routing across multiple size classes.
- Fixed-size allocation mechanics and size-class routing rules belong to different layers of responsibility. 
- Without a clear separation, tests become harder to isolate, API guarantees become mixed together, and documentation has to describe allocation mechanics and routing policy in the same place.

## Decision
Separate fixed-size allocation from size-class routing:
- `SlabAllocator` is responsible for block allocation/deallocation within a single fixed-size class.
- `SlabManager` is responsible for managing multiple size classes and routing allocations to the smallest suitable size class based on the request.

## Consequences

### Pros
- The allocator stays focused on fixed-size block allocation/deallocation, which keeps its responsibilities simpler.
- Management of multiple size classes and request routing is centralized in the manager rather than spread across different allocator instances or pushed to the caller.
- Tests can target allocation mechanics and routing policy separately, API guarantees are easier to assign to the correct layer, and documentation can describe allocator behavior and routing rules without mixing them together.

### Trade-offs
- This adds one more management layer.
- The API and responsibility boundary between the manager and allocator must remain clear so that routing rules do not drift back into the allocator layer.

## Alternatives
- Let the allocator handle both fixed-size block allocation/deallocation and routing across multiple size classes.
  - Not adopted, because this would mix allocation mechanics with routing policy and make the allocator's role unclear.
- Expose multiple allocator instances directly and let the caller choose which size class to use.
  - Not adopted, because this would push class-selection rules out to the caller, increase usage burden, and make contracts more fragmented.