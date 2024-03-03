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
## Without Manual Loop Unrolling

Execution Time (Compiler Optimized): 94 ms

## With Manual Loop Unrolling

Execution Time (Compiler Optimized): 95 ms
*/

#include <iostream>
#include <random>
#include <algorithm>
#include <chrono>
#include <bitset>
#include <functional>
#include <thread>

#include "omp.h"


enum Constants
{
    NUM_OF_SAMPLES = 100'000'000UL,
    NUM_OF_BITS = 32UL,
    SEED = 0xDEADBEEF42UL,
};


#define REVERSE1(RES, VAL, IDX, LOC)    do { RES |= (((VAL) >> (IDX)) & 1U) << (((LOC) - 1U) - ((IDX) >> 1U));                                 } while (false)
#define REVERSE2(RES, VAL, IDX)         do { REVERSE1(RES, VAL, IDX, NUM_OF_BITS); REVERSE1(RES, VAL, IDX + 1U, NUM_OF_BITS >> 1U);            } while (false)
#define REVERSE4(RES, VAL, IDX)         do { REVERSE2(RES, VAL, IDX); REVERSE2(RES, VAL, IDX + 2U);                                            } while (false)
#define REVERSE8(RES, VAL, IDX)         do { REVERSE4(RES, VAL, IDX); REVERSE4(RES, VAL, IDX + 4U);                                            } while (false)
#define REVERSE(RES, VAL)               do { REVERSE8(RES, VAL, 0U); REVERSE8(RES, VAL, 8U); REVERSE8(RES, VAL, 16U); REVERSE8(RES, VAL, 24U); } while (false)


auto inline Setup() noexcept -> void;

auto inline TestSpeed(std::function<void()> const & function, std::string_view const message) noexcept -> void;

auto inline Cooldown(std::chrono::seconds const & seconds = std::chrono::seconds{5}) -> void;

auto inline ReverseBitsNoUnroll() noexcept -> void;
auto inline ReverseBitsManualUnroll() noexcept -> void;

auto inline PrintValues(uint32_t const source, uint32_t const destination) noexcept -> void;

auto inline Cleanup() noexcept -> void;


static uint32_t * source{nullptr};
static uint32_t * destination{nullptr};


auto main() -> int
{
    Setup();
    TestSpeed(ReverseBitsNoUnroll, "reverse bits without manual unrolling");
    Cleanup();

    Cooldown();

    Setup();
    TestSpeed(ReverseBitsManualUnroll, "reverse bits with manual unrolling");
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

auto inline ReverseBitsNoUnroll() noexcept -> void
{
    uint32_t currentValue{0};
    uint32_t currentEvenBit{0};
    uint32_t currentOddBit{0};
    uint32_t reversed{0};

    #pragma omp parallel for
    for (size_t elemIdx = 0; elemIdx < NUM_OF_SAMPLES; ++elemIdx)
    {
        currentValue = source[elemIdx];
        reversed = 0;

        for (size_t bitIdx = 0; bitIdx < NUM_OF_BITS; bitIdx += 2)
        {
            currentEvenBit = (currentValue >> bitIdx) & 1U;
            currentOddBit = (currentValue >> (bitIdx + 1U)) & 1U;

            reversed |= currentEvenBit << ((NUM_OF_BITS - 1U) - (bitIdx >> 1U));
            reversed |= currentOddBit << (((NUM_OF_BITS >> 1U) - 1U) - (bitIdx >> 1U));
        }

        destination[elemIdx] = reversed;
    }
}

auto inline ReverseBitsManualUnroll() noexcept -> void
{
    uint32_t currentValue{0};
    uint32_t reversed{0};

    #pragma omp parallel for
    for (size_t elemIdx = 0; elemIdx < NUM_OF_SAMPLES; ++elemIdx)
    {
        currentValue = source[elemIdx];
        reversed = 0;

        REVERSE(reversed, currentValue);

        destination[elemIdx] = reversed;
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

auto inline Cleanup() noexcept -> void
{
    delete[] source;
    delete[] destination;
}
