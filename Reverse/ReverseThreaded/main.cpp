/*
## Processor

Name: Intel® Core™ i5-6600K
Cores: 4
Threads: 4
Base Frequency: 3.5 GHz
Max Frequency: 3.9 GHz
Cache: 6 MB
Memory Channels: 2
Max Memory Bandwidth: 34.1 GB/s

## Memory

Name: Corsair Vengeance LPX
Type: DDR4
Size: 16 GB (Dual Channel - 2x8 GB)
Speed: 3200 MT/s
Latency (Timings): 16-18-18-36

## Environment

Operating System: Ubuntu 23.10 (Mantic Minotaur)
Kernel: 6.5.0-21-generic
Compiler: gcc 13.2.0
*/

/*
## Threaded Chunked

Execution Time: 1209 ms
Execution Time (Compiler Optimized): 96 ms

## Threaded Chunked - Unrolled

Execution Time: 345 ms
Execution Time (Compiler Optimized): 48 ms

## Threaded Interleaved

Execution Time: 1323 ms
Execution Time (Compiler Optimized): 378 ms

## Threaded Interleaved - Unrolled

Execution Time: 354 ms
Execution Time (Compiler Optimized): 212 ms
*/

#include <iostream>
#include <random>
#include <algorithm>
#include <chrono>
#include <new>
#include <functional>
#include <thread>
#include <bitset>
#include <execution>
#include <ranges>


#define ALIGN    std::align_val_t(std::hardware_destructive_interference_size)

#define INLINE   inline __attribute__((always_inline))

#define NUM_OF_THREADS    std::thread::hardware_concurrency()

enum Constants
{
    NUM_OF_SAMPLES = 100'000'000UL,
    NUM_OF_BITS = 32UL,
    SEED = 0xDEADBEEF42UL,
};

#define REVERSE1(RES, VAL, IDX, LOC)    RES |= (((VAL) >> (IDX)) & 1U) << (((LOC) - 1U) - ((IDX) >> 1U));
#define REVERSE2(RES, VAL, IDX)         REVERSE1(RES, VAL, IDX, NUM_OF_BITS) REVERSE1(RES, VAL, IDX + 1U, NUM_OF_BITS >> 1U)
#define REVERSE4(RES, VAL, IDX)         REVERSE2(RES, VAL, IDX) REVERSE2(RES, VAL, IDX + 2U)
#define REVERSE8(RES, VAL, IDX)         REVERSE4(RES, VAL, IDX) REVERSE4(RES, VAL, IDX + 4U)
#define REVERSE(RES, VAL)               REVERSE8(RES, VAL, 0U) REVERSE8(RES, VAL, 8U) REVERSE8(RES, VAL, 16U) REVERSE8(RES, VAL, 24U)



auto inline Setup() noexcept -> void;

auto inline ReverseBits(size_t const start, size_t const end, size_t const step) -> void;
auto inline ReverseBitsUnrolled(size_t const start, size_t const end, size_t const step) -> void;

auto INLINE ReverseSingleElement(uint32_t * const __restrict dst, uint32_t const * const __restrict src) noexcept -> void;
auto INLINE ReverseSingleElementUnrolled(uint32_t * const __restrict dst, uint32_t const * const __restrict src) noexcept -> void;

auto inline ReverseBitsThreadedChunk(std::function<void(size_t, size_t, size_t)> const & function) noexcept -> void;
auto inline ReverseBitsThreadedInterleaved(std::function<void(size_t, size_t, size_t)> const & function) noexcept -> void;

auto inline PrintValues(uint32_t const source, uint32_t const destination) noexcept -> void;

auto inline TestSpeed(std::function<void()> const & function, std::string_view const message) noexcept -> void;

auto inline Cooldown(std::chrono::seconds const & seconds = std::chrono::seconds{5}) -> void;

auto inline Check(void const * const ptr, std::string_view const message) noexcept -> void;

auto inline Cleanup() noexcept -> void;


static uint32_t * source{nullptr};
static uint32_t * destination{nullptr};


auto main() -> int
{
    Setup();
    TestSpeed([&]() -> void { ReverseBitsThreadedChunk(ReverseBits); }, "ReverseBitsThreadedChunk");
    Cooldown();
    TestSpeed([&]() -> void { ReverseBitsThreadedChunk(ReverseBitsUnrolled); }, "ReverseBitsThreadedChunk (Unrolled)");
    Cleanup();

    Cooldown();

    Setup();
    TestSpeed([&]() -> void { ReverseBitsThreadedInterleaved(ReverseBits); }, "ReverseBitsThreadedInterleaved");
    Cooldown();
    TestSpeed([&]() -> void { ReverseBitsThreadedInterleaved(ReverseBitsUnrolled); }, "ReverseBitsThreadedInterleaved (Unrolled)");
    Cleanup();

    return 0;
}

auto inline Setup() noexcept -> void
{
    source = new(ALIGN, std::nothrow) uint32_t[NUM_OF_SAMPLES];
    Check(source, "source array");

    destination = new(ALIGN, std::nothrow) uint32_t[NUM_OF_SAMPLES];
    Check(destination, "destination array");

    std::mt19937 randomEngine{SEED};
    std::uniform_int_distribution<uint32_t> randomDistribution{0, std::numeric_limits<uint32_t>::max()};

    auto generator = [&]() -> uint32_t { return randomDistribution(randomEngine); };
    std::generate(source, source + NUM_OF_SAMPLES, generator);
}

auto inline ReverseBits(size_t const start, size_t const end, size_t const step) -> void
{
    for (register size_t elemIdx = start; elemIdx < end; elemIdx += step)
    {
        ReverseSingleElement(destination + elemIdx, source + elemIdx);
    }
}

auto inline ReverseBitsUnrolled(size_t const start, size_t const end, size_t const step) -> void
{
    for (register size_t elemIdx = start; elemIdx < end; elemIdx += step)
    {
        ReverseSingleElementUnrolled(destination + elemIdx, source + elemIdx);
    }
}

auto INLINE ReverseSingleElement(uint32_t * const __restrict dst, uint32_t const * const __restrict src) noexcept -> void
{
    register uint32_t currentValue{*src};
    register uint32_t currentEvenBit{0};
    register uint32_t currentOddBit{0};
    register uint32_t reversed{0};

    for (register size_t bitIdx = 0; bitIdx < NUM_OF_BITS; bitIdx += 2)
    {
        currentEvenBit = (currentValue >> bitIdx) & 1U;
        currentOddBit = (currentValue >> (bitIdx + 1U)) & 1U;

        reversed |= currentEvenBit << ((NUM_OF_BITS - 1U) - (bitIdx >> 1U));
        reversed |= currentOddBit << (((NUM_OF_BITS >> 1U) - 1U) - (bitIdx >> 1U));
    }

    *dst = reversed;
}

auto INLINE ReverseSingleElementUnrolled(uint32_t * const __restrict dst, uint32_t const * const __restrict src) noexcept -> void
{
    register uint32_t currentValue{*src};
    register uint32_t reversed{0};

    REVERSE(reversed, currentValue)

    *dst = reversed;
}

auto inline ReverseBitsThreadedChunk(std::function<void(size_t, size_t, size_t)> const & function) noexcept -> void
{
    std::vector<std::unique_ptr<std::thread>> threads;

    size_t start{0};
    size_t end{0};

    for (size_t threadIdx = 0; threadIdx < NUM_OF_THREADS; ++threadIdx)
    {
        start = threadIdx * (NUM_OF_SAMPLES / NUM_OF_THREADS);
        end = start + (NUM_OF_SAMPLES / NUM_OF_THREADS);
        threads.emplace_back(std::make_unique<std::thread>(function, start, end, 1));
    }

    for (auto const & thread: threads)
    {
        thread->join();
    }
}

auto inline ReverseBitsThreadedInterleaved(std::function<void(size_t, size_t, size_t)> const & function) noexcept -> void
{
    std::vector<std::unique_ptr<std::thread>> threads;

    for (size_t threadIdx = 0; threadIdx < NUM_OF_THREADS; ++threadIdx)
    {
        threads.emplace_back(std::make_unique<std::thread>(function, threadIdx, NUM_OF_SAMPLES, NUM_OF_THREADS));
    }

    for (auto const & thread: threads)
    {
        thread->join();
    }
}

auto inline PrintValues(uint32_t const source, uint32_t const destination) noexcept -> void
{
    static std::bitset<NUM_OF_BITS> sourceBits;
    static std::bitset<NUM_OF_BITS> destinationBits;

    sourceBits = source;
    destinationBits = destination;

    printf("Source:      %s (0x%08X)\n", sourceBits.to_string().c_str(), source);
    printf("Destination: %s (0x%08X)\n", destinationBits.to_string().c_str(), destination);
}

auto inline TestSpeed(std::function<void()> const & function, std::string_view const message) noexcept -> void
{
    auto const start = std::chrono::high_resolution_clock::now();
    function();
    auto const stop = std::chrono::high_resolution_clock::now();

    auto const difference_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    auto const time_ms = difference_ms.count();

    std::cout << "Time taken for " << message << " : " << time_ms << " ms\n";
}

auto inline Cooldown(std::chrono::seconds const & seconds) -> void
{
    std::this_thread::sleep_for(seconds);
}

auto inline Check(void const * const ptr, std::string_view const message) noexcept -> void
{
    if (ptr == nullptr)
    {
        std::cerr << "Failed to allocate memory for the " << message << ".\n";
        Cleanup();
        std::exit(EXIT_FAILURE);
    }
}

auto inline Cleanup() noexcept -> void
{
    delete[] source;
    delete[] destination;
}
