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
## 32-bit LUTs

Space: 16 GiB
Execution Time (Compiler Optimized): Out of Memory

## 16-bit LUTs

Space: 512 KiB
Execution Time (Compiler Optimized): 109 ms

## 8-bit LUTs

Space: 4 KiB
Execution Time (Compiler Optimized): 79 ms

## 4-bit LUTs

Space: 512 B
Execution Time (Compiler Optimized): 110 ms
*/

#include <iostream>
#include <random>
#include <algorithm>
#include <chrono>
#include <functional>
#include <bitset>
#include <thread>

#include "omp.h"


#define UINT32(val)    static_cast<uint32_t>(val)

#define LUT_SIZE_32       ( std::numeric_limits<uint32_t>::max() + 1 )
#define NUM_OF_BITS_32    ( 32U )

#define LUT_SIZE_16       ( std::numeric_limits<uint16_t>::max() + 1 )

#define LUT_SIZE_8        ( std::numeric_limits<uint8_t>::max() + 1 )

#define LUT_SIZE_4        ( 16U )


enum Constants
{
    NUM_OF_SAMPLES = 100'000'000L,
    SEED = 0xDEADBEEF42UL,
};


auto inline Setup() noexcept -> void;

auto inline Build32BitLut() noexcept -> void;
auto inline Build16BitLut() noexcept -> void;
auto inline Build8BitLut() noexcept -> void;
auto inline Build4BitLut() noexcept -> void;

auto inline ReverseBits(size_t const element) noexcept -> uint32_t;

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


static uint32_t * lut32{nullptr};

static uint32_t * lut16_word1{nullptr};
static uint32_t * lut16_word0{nullptr};

static uint32_t * lut8_byte3{nullptr};
static uint32_t * lut8_byte2{nullptr};
static uint32_t * lut8_byte1{nullptr};
static uint32_t * lut8_byte0{nullptr};

static uint32_t * lut4_nibble7{nullptr};
static uint32_t * lut4_nibble6{nullptr};
static uint32_t * lut4_nibble5{nullptr};
static uint32_t * lut4_nibble4{nullptr};
static uint32_t * lut4_nibble3{nullptr};
static uint32_t * lut4_nibble2{nullptr};
static uint32_t * lut4_nibble1{nullptr};
static uint32_t * lut4_nibble0{nullptr};


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
    lut32 = new(std::nothrow) uint32_t[LUT_SIZE_32];
    Check(lut32, "32-bit lookup table");

    #pragma omp parallel for
    for (size_t elem = 0; elem < LUT_SIZE_32; ++elem)
    {
        lut32[elem] = ReverseBits(elem);
    }
}

auto inline Build16BitLut() noexcept -> void
{
    lut16_word1 = new(std::nothrow) uint32_t[LUT_SIZE_16];
    Check(lut16_word1, "16-bit lookup table (word1)");

    lut16_word0 = new(std::nothrow) uint32_t[LUT_SIZE_16];
    Check(lut16_word0, "16-bit lookup table (word0)");

    #pragma omp parallel for
    for (size_t elem = 0; elem < LUT_SIZE_16; ++elem)
    {
        lut16_word1[elem] = ReverseBits(UINT32(elem) << 16U);
        lut16_word0[elem] = ReverseBits(UINT32(elem) << 0U);
    }
}

auto inline Build8BitLut() noexcept -> void
{
    lut8_byte3 = new(std::nothrow) uint32_t[LUT_SIZE_8];
    Check(lut8_byte3, "8-bit lookup table (byte3)");

    lut8_byte2 = new(std::nothrow) uint32_t[LUT_SIZE_8];
    Check(lut8_byte2, "8-bit lookup table (byte2)");

    lut8_byte1 = new(std::nothrow) uint32_t[LUT_SIZE_8];
    Check(lut8_byte1, "8-bit lookup table (byte1)");

    lut8_byte0 = new(std::nothrow) uint32_t[LUT_SIZE_8];
    Check(lut8_byte0, "8-bit lookup table (byte0)");

    #pragma omp parallel for
    for (size_t elem = 0; elem < LUT_SIZE_8; ++elem)
    {
        lut8_byte3[elem] = ReverseBits(UINT32(elem) << 24U);
        lut8_byte2[elem] = ReverseBits(UINT32(elem) << 16U);
        lut8_byte1[elem] = ReverseBits(UINT32(elem) << 8U);
        lut8_byte0[elem] = ReverseBits(UINT32(elem) << 0U);
    }
}

auto inline Build4BitLut() noexcept -> void
{
    lut4_nibble7 = new(std::nothrow) uint32_t[LUT_SIZE_4];
    Check(lut4_nibble7, "4-bit lookup table (nibble7)");

    lut4_nibble6 = new(std::nothrow) uint32_t[LUT_SIZE_4];
    Check(lut4_nibble6, "4-bit lookup table (nibble6)");

    lut4_nibble5 = new(std::nothrow) uint32_t[LUT_SIZE_4];
    Check(lut4_nibble5, "4-bit lookup table (nibble5)");

    lut4_nibble4 = new(std::nothrow) uint32_t[LUT_SIZE_4];
    Check(lut4_nibble4, "4-bit lookup table (nibble4)");

    lut4_nibble3 = new(std::nothrow) uint32_t[LUT_SIZE_4];
    Check(lut4_nibble3, "4-bit lookup table (nibble3)");

    lut4_nibble2 = new(std::nothrow) uint32_t[LUT_SIZE_4];
    Check(lut4_nibble2, "4-bit lookup table (nibble2)");

    lut4_nibble1 = new(std::nothrow) uint32_t[LUT_SIZE_4];
    Check(lut4_nibble1, "4-bit lookup table (nibble1)");

    lut4_nibble0 = new(std::nothrow) uint32_t[LUT_SIZE_4];
    Check(lut4_nibble0, "4-bit lookup table (nibble0)");

    #pragma omp parallel for
    for (size_t elem = 0; elem < LUT_SIZE_4; ++elem)
    {
        lut4_nibble7[elem] = ReverseBits(UINT32(elem) << 28U);
        lut4_nibble6[elem] = ReverseBits(UINT32(elem) << 24U);
        lut4_nibble5[elem] = ReverseBits(UINT32(elem) << 20U);
        lut4_nibble4[elem] = ReverseBits(UINT32(elem) << 16U);
        lut4_nibble3[elem] = ReverseBits(UINT32(elem) << 12U);
        lut4_nibble2[elem] = ReverseBits(UINT32(elem) << 8U);
        lut4_nibble1[elem] = ReverseBits(UINT32(elem) << 4U);
        lut4_nibble0[elem] = ReverseBits(UINT32(elem) << 0U);
    }
}

auto inline ReverseBits(size_t const element) noexcept -> uint32_t
{
    uint32_t currentEvenBit{0};
    uint32_t currentOddBit{0};
    uint32_t reversed{0};

    for (size_t bitIdx = 0; bitIdx < NUM_OF_BITS_32; bitIdx += 2)
    {
        currentEvenBit = (element >> bitIdx) & 1U;
        currentOddBit = (element >> (bitIdx + 1U)) & 1U;

        reversed |= currentEvenBit << ((NUM_OF_BITS_32 - 1U) - (bitIdx >> 1U));
        reversed |= currentOddBit << (((NUM_OF_BITS_32 >> 1U) - 1U) - (bitIdx >> 1U));
    }

    return reversed;
}

auto inline ReverseBits32BitLut() noexcept -> void
{
    #pragma omp parallel for
    for (size_t elemIdx = 0; elemIdx < NUM_OF_SAMPLES; ++elemIdx)
    {
        destination[elemIdx] = lut32[source[elemIdx]];
    }
}

auto inline ReverseBits16BitLut() noexcept -> void
{
    #pragma omp parallel for
    for (size_t elemIdx = 0; elemIdx < NUM_OF_SAMPLES; ++elemIdx)
    {
        destination[elemIdx] = lut16_word1[(source[elemIdx] >> 16U) & 0xFFFFU] | lut16_word0[(source[elemIdx] >> 0U) & 0xFFFFU];
    }
}

auto inline ReverseBits8BitLut() noexcept -> void
{
    #pragma omp parallel for
    for (size_t elemIdx = 0; elemIdx < NUM_OF_SAMPLES; ++elemIdx)
    {
        destination[elemIdx] = lut8_byte3[(source[elemIdx] >> 24U) & 0xFFU] | lut8_byte2[(source[elemIdx] >> 16U) & 0xFFU] |
                               lut8_byte1[(source[elemIdx] >> 8U) & 0xFFU] | lut8_byte0[(source[elemIdx] >> 0U) & 0xFFU];
    }
}

auto inline ReverseBits4BitLut() noexcept -> void
{
    #pragma omp parallel for
    for (size_t elemIdx = 0; elemIdx < NUM_OF_SAMPLES; ++elemIdx)
    {
        destination[elemIdx] = lut4_nibble7[(source[elemIdx] >> 28U) & 0xFU] | lut4_nibble6[(source[elemIdx] >> 24U) & 0xFU] |
                               lut4_nibble5[(source[elemIdx] >> 20U) & 0xFU] | lut4_nibble4[(source[elemIdx] >> 16U) & 0xFU] |
                               lut4_nibble3[(source[elemIdx] >> 12U) & 0xFU] | lut4_nibble2[(source[elemIdx] >> 8U) & 0xFU] |
                               lut4_nibble1[(source[elemIdx] >> 4U) & 0xFU] | lut4_nibble0[(source[elemIdx] >> 0U) & 0xFU];
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

    delete[] lut16_word1;
    lut16_word1 = nullptr;
    delete[] lut16_word0;
    lut16_word0 = nullptr;

    delete[] lut8_byte3;
    lut8_byte3 = nullptr;
    delete[] lut8_byte2;
    lut8_byte2 = nullptr;
    delete[] lut8_byte1;
    lut8_byte1 = nullptr;
    delete[] lut8_byte0;
    lut8_byte0 = nullptr;

    delete[] lut4_nibble7;
    lut4_nibble7 = nullptr;
    delete[] lut4_nibble6;
    lut4_nibble6 = nullptr;
    delete[] lut4_nibble5;
    lut4_nibble5 = nullptr;
    delete[] lut4_nibble4;
    lut4_nibble4 = nullptr;
    delete[] lut4_nibble3;
    lut4_nibble3 = nullptr;
    delete[] lut4_nibble2;
    lut4_nibble2 = nullptr;
    delete[] lut4_nibble1;
    lut4_nibble1 = nullptr;
    delete[] lut4_nibble0;
    lut4_nibble0 = nullptr;
}
