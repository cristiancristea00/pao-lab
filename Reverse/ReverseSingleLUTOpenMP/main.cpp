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
## 32-bit LUT

Space: 16 GiB
Execution Time (Compiler Optimized): Out of Memory

## 16-bit LUT

Space: 128 KiB
Execution Time (Compiler Optimized): 468 ms

## 8-bit LUT

Space: 256 B
Execution Time (Compiler Optimized): 669 ms

## 4-bit LUT

Space: 8 B
Execution Time (Compiler Optimized): 1186 ms
*/

#include <iostream>
#include <random>
#include <algorithm>
#include <chrono>
#include <functional>
#include <bitset>
#include <thread>

#include "omp.h"

#include "lut_types.h"

#define UINT32(val)    static_cast<uint32_t>(val)

#define LUT_SIZE_32       ( std::numeric_limits<uint32_t>::max() + 1 )
#define NUM_OF_BITS_32    ( 32U )

#define LUT_SIZE_16       ( std::numeric_limits<uint16_t>::max() + 1 )
#define NUM_OF_BITS_16    ( 16U )

#define LUT_SIZE_8        ( std::numeric_limits<uint8_t>::max() + 1 )
#define NUM_OF_BITS_8     ( 8U )

#define LUT_SIZE_4        ( 16U )
#define NUM_OF_BITS_4     ( 4U )


enum Constants
{
    NUM_OF_SAMPLES = 100'000'000UL,
    SEED = 0xDEADBEEF42UL,
};


auto inline Setup() noexcept -> void;

auto inline Build32BitLut() noexcept -> void;
auto inline Build16BitLut() noexcept -> void;
auto inline Build8BitLut() noexcept -> void;
auto inline Build4BitLut() noexcept -> void;

template <typename T>
auto inline ReverseBits(size_t const element, size_t const numOfBits) noexcept -> T;

auto inline ReverseBits32BitLut() noexcept -> void;
auto inline ReverseBits16BitLut() noexcept -> void;
auto inline ReverseBits8BitLut() noexcept -> void;
auto inline ReverseBits4BitLut() noexcept -> void;

auto inline PrintValues(uint32_t const source, uint32_t const destination) noexcept -> void;

auto inline TestSpeed(std::function<void()> const & function, std::string_view const message) noexcept -> void;

auto inline Cooldown(std::chrono::seconds const & seconds = std::chrono::seconds{5}) -> void;

auto inline Check(void const * const ptr, std::string_view const message) noexcept -> void;

auto inline Cleanup() noexcept -> void;


static uint32_t * source{nullptr};
static uint32_t * destination{nullptr};


static lut32_t * lut32{nullptr};

static lut16_t * lut16{nullptr};

static lut8_t * lut8{nullptr};

static lut4_t * lut4{nullptr};


auto main() -> int
{
    Setup();
    TestSpeed(Build32BitLut, "32-bit LUT creation");
    TestSpeed(ReverseBits32BitLut, "32-bit LUT reversal");
    Cleanup();

    Cooldown();

    Setup();
    TestSpeed(Build16BitLut, "16-bit LUT creation");
    TestSpeed(ReverseBits16BitLut, "16-bit LUT reversal");
    Cleanup();

    Cooldown();

    Setup();
    TestSpeed(Build8BitLut, "8-bit LUT creation");
    TestSpeed(ReverseBits8BitLut, "8-bit LUT reversal");
    Cleanup();

    Cooldown();

    Setup();
    TestSpeed(Build4BitLut, "4-bit LUT creation");
    TestSpeed(ReverseBits4BitLut, "4-bit LUT reversal");
    Cleanup();

    return 0;
}

auto inline Setup() noexcept -> void
{
    source = new(std::nothrow) uint32_t[NUM_OF_SAMPLES];
    Check(source, "source array");

    destination = new(std::nothrow) uint32_t[NUM_OF_SAMPLES];
    Check(destination, "destination array");

    std::mt19937 randomEngine{SEED};
    std::uniform_int_distribution<uint32_t> randomDistribution{0, std::numeric_limits<uint32_t>::max()};

    auto generator = [&]() -> uint32_t { return randomDistribution(randomEngine); };
    std::generate(source, source + NUM_OF_SAMPLES, generator);
}

auto inline Build32BitLut() noexcept -> void
{
    lut32 = new(std::nothrow) lut32_t[LUT_SIZE_32];
    Check(lut32, "32-bit lookup table");

    #pragma omp parallel for
    for (size_t elem = 0; elem < LUT_SIZE_32; ++elem)
    {
        lut32[elem].value = ReverseBits<uint32_t>(elem, NUM_OF_BITS_32);
    }
}

auto inline Build16BitLut() noexcept -> void
{
    lut16 = new(std::nothrow) lut16_t[LUT_SIZE_16];
    Check(lut16, "16-bit lookup table");

    #pragma omp parallel for
    for (size_t elem = 0; elem < LUT_SIZE_16; ++elem)
    {
        lut16[elem].value = ReverseBits<uint16_t>(elem, NUM_OF_BITS_16);
    }
}

auto inline Build8BitLut() noexcept -> void
{
    lut8 = new(std::nothrow) lut8_t[LUT_SIZE_8];
    Check(lut8, "8-bit lookup table");

    #pragma omp parallel for
    for (size_t elem = 0; elem < LUT_SIZE_8; ++elem)
    {
        lut8[elem].value = ReverseBits<uint8_t>(elem, NUM_OF_BITS_8);
    }
}

auto inline Build4BitLut() noexcept -> void
{
    lut4 = new(std::nothrow) lut4_t[LUT_SIZE_4];
    Check(lut4, "4-bit lookup table");

    #pragma omp parallel for
    for (size_t elem = 0; elem < LUT_SIZE_4; ++elem)
    {
        lut4[elem].value = ReverseBits<uint8_t>(elem, NUM_OF_BITS_4);
    }
}

template <typename T>
auto inline ReverseBits(size_t const element, size_t const numOfBits) noexcept -> T
{
    T currentEvenBit{0};
    T currentOddBit{0};
    T reversed{0};

    for (size_t bitIdx = 0; bitIdx < numOfBits; bitIdx += 2)
    {
        currentEvenBit = (element >> bitIdx) & 1U;
        currentOddBit = (element >> (bitIdx + 1U)) & 1U;

        reversed |= currentEvenBit << ((numOfBits - 1U) - (bitIdx >> 1U));
        reversed |= currentOddBit << (((numOfBits >> 1U) - 1U) - (bitIdx >> 1U));
    }

    return reversed;
}

auto inline ReverseBits32BitLut() noexcept -> void
{
    #pragma omp parallel for
    for (size_t elemIdx = 0; elemIdx < NUM_OF_SAMPLES; ++elemIdx)
    {
        destination[elemIdx] = lut32[source[elemIdx]].value;
    }
}

auto inline ReverseBits16BitLut() noexcept -> void
{
    uint32_t currentValue{0};

    lut16_t word1{0};
    lut16_t word0{0};

    #pragma omp parallel for
    for (size_t elemIdx = 0; elemIdx < NUM_OF_SAMPLES; ++elemIdx)
    {
        currentValue = source[elemIdx];

        word1 = lut16[(currentValue >> 16U) & 0xFFFFU];
        word0 = lut16[(currentValue >> 0U) & 0xFFFFU];

        destination[elemIdx] = (UINT32(word0.msb8) << 24U) | (UINT32(word1.msb8) << 16U) |
                               (UINT32(word0.lsb8) << 8U) | (UINT32(word1.lsb8) << 0U);
    }
}

auto inline ReverseBits8BitLut() noexcept -> void
{
    uint32_t currentValue{0};

    lut8_t byte3{0};
    lut8_t byte2{0};
    lut8_t byte1{0};
    lut8_t byte0{0};

    #pragma omp parallel for
    for (size_t elemIdx = 0; elemIdx < NUM_OF_SAMPLES; ++elemIdx)
    {
        currentValue = source[elemIdx];

        byte3 = lut8[(currentValue >> 24U) & 0xFFU];
        byte2 = lut8[(currentValue >> 16U) & 0xFFU];
        byte1 = lut8[(currentValue >> 8U) & 0xFFU];
        byte0 = lut8[(currentValue >> 0U) & 0xFFU];

        destination[elemIdx] = (UINT32(byte0.msb4) << 28U) | (UINT32(byte1.msb4) << 24U) |
                               (UINT32(byte2.msb4) << 20U) | (UINT32(byte3.msb4) << 16U) |
                               (UINT32(byte0.lsb4) << 12U) | (UINT32(byte1.lsb4) << 8U) |
                               (UINT32(byte2.lsb4) << 4U) | (UINT32(byte3.lsb4) << 0U);
    }
}

auto inline ReverseBits4BitLut() noexcept -> void
{
    uint32_t currentValue{0};

    lut4_t nibble7{0};
    lut4_t nibble6{0};
    lut4_t nibble5{0};
    lut4_t nibble4{0};
    lut4_t nibble3{0};
    lut4_t nibble2{0};
    lut4_t nibble1{0};
    lut4_t nibble0{0};

    #pragma omp parallel for
    for (size_t elemIdx = 0; elemIdx < NUM_OF_SAMPLES; ++elemIdx)
    {
        currentValue = source[elemIdx];

        nibble7 = lut4[(currentValue >> 28U) & 0xFU];
        nibble6 = lut4[(currentValue >> 24U) & 0xFU];
        nibble5 = lut4[(currentValue >> 20U) & 0xFU];
        nibble4 = lut4[(currentValue >> 16U) & 0xFU];
        nibble3 = lut4[(currentValue >> 12U) & 0xFU];
        nibble2 = lut4[(currentValue >> 8U) & 0xFU];
        nibble1 = lut4[(currentValue >> 4U) & 0xFU];
        nibble0 = lut4[(currentValue >> 0U) & 0xFU];

        destination[elemIdx] = (UINT32(nibble0.msb2) << 30U) | (UINT32(nibble1.msb2) << 28U) |
                               (UINT32(nibble2.msb2) << 26U) | (UINT32(nibble3.msb2) << 24U) |
                               (UINT32(nibble4.msb2) << 22U) | (UINT32(nibble5.msb2) << 20U) |
                               (UINT32(nibble6.msb2) << 18U) | (UINT32(nibble7.msb2) << 16U) |
                               (UINT32(nibble0.lsb2) << 14U) | (UINT32(nibble1.lsb2) << 12U) |
                               (UINT32(nibble2.lsb2) << 10U) | (UINT32(nibble3.lsb2) << 8U) |
                               (UINT32(nibble4.lsb2) << 6U) | (UINT32(nibble5.lsb2) << 4U) |
                               (UINT32(nibble6.lsb2) << 2U) | (UINT32(nibble7.lsb2) << 0U);
    }
}

auto inline PrintValues(uint32_t const source, uint32_t const destination) noexcept -> void
{
    static std::bitset<NUM_OF_BITS_32> sourceBits;
    static std::bitset<NUM_OF_BITS_32> destinationBits;

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
    source = nullptr;
    delete[] destination;
    destination = nullptr;

    delete[] lut32;
    lut32 = nullptr;
    delete[] lut16;
    lut16 = nullptr;
    delete[] lut8;
    lut8 = nullptr;
    delete[] lut4;
    lut4 = nullptr;
}
