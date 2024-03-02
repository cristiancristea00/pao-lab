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
Execution Time: 5634 ms

Execution Time (Compiler Optimized): 3030 ms
*/

#include <iostream>
#include <random>
#include <algorithm>
#include <chrono>


enum Constants
{
    NUM_OF_SAMPLES = 100'000'000UL,
    NUM_OF_BITS = 32UL,
    SEED = 0xDEADBEEF42UL,
};


auto inline Setup() noexcept -> void;

auto inline TestSpeed() noexcept -> void;

auto inline ReverseBits() noexcept -> void;

auto inline Cleanup() noexcept -> void;


static uint32_t * source{nullptr};
static uint32_t * destination{nullptr};


auto main() -> int
{
    Setup();
    TestSpeed();
    Cleanup();

    return 0;
}

auto inline Setup() noexcept -> void
{
    source = new(std::nothrow) uint32_t[NUM_OF_SAMPLES];
    destination = new(std::nothrow) uint32_t[NUM_OF_SAMPLES];

    if (source == nullptr)
    {
        std::cerr << "Failed to allocate memory for the source array.\n";
        Cleanup();
        std::exit(EXIT_FAILURE);
    }

    if (destination == nullptr)
    {
        std::cerr << "Failed to allocate memory for the destination array.\n";
        Cleanup();
        std::exit(EXIT_FAILURE);
    }

    std::mt19937 randomEngine{SEED};
    std::uniform_int_distribution<uint32_t> randomDistribution{0, std::numeric_limits<uint32_t>::max()};

    auto generator = [&]() -> uint32_t { return randomDistribution(randomEngine); };
    std::generate(source, source + NUM_OF_SAMPLES, generator);
}

auto inline TestSpeed() noexcept -> void
{
    auto const start = std::chrono::high_resolution_clock::now();
    ReverseBits();
    auto const stop = std::chrono::high_resolution_clock::now();

    auto const difference_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    auto const time_ms = difference_ms.count();

    std::cout << "Time taken: " << time_ms << " ms";
}

auto inline ReverseBits() noexcept -> void
{
    register uint32_t currentValue{0};
    register uint32_t currentBit{0};
    register uint32_t reversed{0};

    for (register size_t elemIdx = 0; elemIdx < NUM_OF_SAMPLES; ++elemIdx)
    {
        currentValue = source[elemIdx];
        reversed = 0;

        for (register size_t bitIdx = 0; bitIdx < NUM_OF_BITS; ++bitIdx)
        {
            currentBit = (currentValue >> bitIdx) & 1U;

            if ((bitIdx & 1U) == 0U)
            {
                reversed |= currentBit << ((NUM_OF_BITS - 1U) - (bitIdx >> 1U));
            }
            else
            {
                reversed |= currentBit << (((NUM_OF_BITS >> 1U) - 1U) - (bitIdx >> 1U));
            }
        }

        destination[elemIdx] = reversed;
    }
}

auto inline Cleanup() noexcept -> void
{
    delete[] source;
    delete[] destination;
}
