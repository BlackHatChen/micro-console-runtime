#include <benchmark/benchmark.h>
#include <slab_allocator.h>
#include <cstdlib>

namespace {
    constexpr std::size_t kObjectSize = 24;
    constexpr std::size_t kBatchSize = 1000; // Batch allocate/free memory counts.

    // Benchmark 1: OS standard library std::malloc / std::free
    void BM_SystemMalloc(benchmark::State& state) {
        std::vector<void*> pointers;
        pointers.reserve(kBatchSize);

        // State loop will decide how many times should run until get the stable analysis data.
        for (auto _ : state) {
            // Batch allocate.
            for (std::size_t i = 0; i < kBatchSize; i++) {
                void* ptr = std::malloc(kObjectSize);
                // Prevent Dead Code Elimination.
                benchmark::DoNotOptimize(ptr);
                pointers.push_back(ptr);
            }
            
            // Batch deallocate.
            for (void* ptr : pointers) {
                std::free(ptr);
            }
            pointers.clear();
        }
    }
    // Register the test.
    BENCHMARK(BM_SystemMalloc);

    // Benchmark 2: Slab Allocator (O(1) allocate/free).
    void BM_SlabAllocator(benchmark::State& state) {
        const std::size_t pool_size = kObjectSize * kBatchSize;
        mcr::SlabAllocator allocator(kObjectSize, pool_size);

        std::vector<void*> pointers;
        pointers.reserve(kBatchSize);

        for (auto _ : state) {
            // Batch allocate.
            for (std::size_t i = 0; i < kBatchSize; i++) {
                void* ptr = allocator.Allocate();
                benchmark::DoNotOptimize(ptr);
                pointers.push_back(ptr);
            }
            
            // Batch free.
            for (void* ptr : pointers) {
                allocator.Free(ptr);
            }
            pointers.clear();
        }
    }
    BENCHMARK(BM_SlabAllocator);
}

BENCHMARK_MAIN(); // main function macro provided by Google Benchmark.