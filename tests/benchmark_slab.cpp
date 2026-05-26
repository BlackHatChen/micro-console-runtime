#include <benchmark/benchmark.h>
#include <slab_allocator.h>
#include <cstddef>
#include <cstdlib>
#include <vector>

namespace
{
    constexpr std::size_t kObjectSize = 24;
    constexpr std::size_t kBatchSize = 1000;

    constexpr std::size_t AlignUp(std::size_t value, std::size_t alignment)
    {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    constexpr std::size_t EffectiveSlabBlockSize()
    {
        constexpr std::size_t alignment = sizeof(void *);
        const std::size_t block_size = (kObjectSize > sizeof(void *)) ? kObjectSize : sizeof(void *);
        return AlignUp(block_size, alignment);
    }

    // Benchmark 1: OS Standard Library (malloc / free)
    void BM_SystemMalloc(benchmark::State &state)
    {
        std::vector<void *> pointers;
        pointers.reserve(kBatchSize);

        // State loop decides how many times should run until get the stable analysis data.
        for (auto _ : state)
        {
            // Batch allocate.
            for (std::size_t i = 0; i < kBatchSize; i++)
            {
                void *ptr = std::malloc(kObjectSize);
                // Prevent Dead Code Elimination.
                benchmark::DoNotOptimize(ptr);
                pointers.push_back(ptr);
            }

            // Batch deallocate.
            for (void *ptr : pointers)
            {
                std::free(ptr);
            }
            pointers.clear();
        }
    }
    // Register the test.
    BENCHMARK(BM_SystemMalloc);

    // Benchmark 2: Slab Allocator (O(1) `Allocate`/`Free`).
    void BM_SlabAllocator(benchmark::State &state)
    {
        const std::size_t pool_size = EffectiveSlabBlockSize() * kBatchSize;
        mcr::SlabAllocator allocator(kObjectSize, pool_size);

        std::vector<void *> pointers;
        pointers.reserve(kBatchSize);

        for (auto _ : state)
        {
            // Batch allocate.
            for (std::size_t i = 0; i < kBatchSize; i++)
            {
                void *ptr = allocator.Allocate();
                if (!ptr)
                {
                    state.SkipWithError("SlabAllocator exhausted during benchmark batch.");

                    // Batch deallocate early. (Because SlabAllocator exhausted already.)
                    for (void *allocated_ptr : pointers)
                    {
                        allocator.Free(allocated_ptr);
                    }
                    pointers.clear();
                    return;
                }

                benchmark::DoNotOptimize(ptr);
                pointers.push_back(ptr);
            }

            // Batch deallocate.
            for (void *ptr : pointers)
            {
                allocator.Free(ptr);
            }
            pointers.clear();
        }
    }
    // Register the test.
    BENCHMARK(BM_SlabAllocator);
}
