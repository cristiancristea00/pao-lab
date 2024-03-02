#include <iostream>
#include <random>
#include <algorithm>
#include <chrono>
#include <functional>
#include <bitset>
#include <thread>

#include "lut_types.h"

#define UINT32(val)    static_cast<uint32_t>(val)


enum Constants
{
    NUM_OF_SAMPLES = 100'000'000UL,
    SEED = 0xDEADBEEF42UL,
};

enum LutConstants : size_t
{
    LUT_MAX_32 = std::numeric_limits<uint32_t>::max(),
    LUT_SIZE_32 = LUT_MAX_32 + 1,
    NUM_OF_BITS_32 = 32U,

    LUT_MAX_16 = std::numeric_limits<uint16_t>::max(),
    LUT_SIZE_16 = LUT_MAX_16 + 1,
    NUM_OF_BITS_16 = 16U,

    LUT_MAX_8 = std::numeric_limits<uint8_t>::max(),
    LUT_SIZE_8 = LUT_MAX_8 + 1,
    NUM_OF_BITS_8 = 8U,

    LUT_MAX_4 = 15UL,
    LUT_SIZE_4 = LUT_MAX_4 + 1,
    NUM_OF_BITS_4 = 4U,
};


auto inline Setup() noexcept -> void;

auto inline Build32BitLut() noexcept -> void;
auto inline Build16BitLut() noexcept -> void;
auto inline Build8BitLut() noexcept -> void;
auto inline Build4BitLut() noexcept -> void;

auto inline ReverseBits32BitLut() noexcept -> void;
auto inline ReverseBits16BitLut() noexcept -> void;
auto inline ReverseBits8BitLut() noexcept -> void;
auto inline ReverseBits4BitLut() noexcept -> void;

auto inline TestSpeed(std::function<void()> const & function, std::string_view const message) noexcept -> void;

auto inline Cooldown(std::chrono::seconds const & seconds = std::chrono::seconds{5}) -> void;

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

    if (source == nullptr)
    {
        std::cerr << "Failed to allocate memory for the source array.\n";
        Cleanup();
        std::exit(EXIT_FAILURE);
    }

    destination = new(std::nothrow) uint32_t[NUM_OF_SAMPLES];

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

auto inline Build32BitLut() noexcept -> void
{
    lut32 = new(std::nothrow) lut32_t[LUT_SIZE_32];

    if (lut32 == nullptr)
    {
        std::cerr << "Failed to allocate memory for the 32-bit LUT.\n";
        Cleanup();
        std::exit(EXIT_FAILURE);
    }

    register uint32_t currentBit{0};
    register uint32_t reversed{0};

    for (register size_t elem = 0; elem < LUT_SIZE_32; ++elem)
    {
        reversed = 0;

        for (register size_t bitIdx = 0; bitIdx < NUM_OF_BITS_32; ++bitIdx)
        {
            currentBit = (elem >> bitIdx) & 1U;

            if ((bitIdx & 1U) == 0U)
            {
                reversed |= currentBit << ((NUM_OF_BITS_32 - 1U) - (bitIdx >> 1U));
            }
            else
            {
                reversed |= currentBit << (((NUM_OF_BITS_32 >> 1U) - 1U) - (bitIdx >> 1U));
            }
        }

        lut32[elem].value = reversed;
    }
}

auto inline Build16BitLut() noexcept -> void
{
    lut16 = new(std::nothrow) lut16_t[LUT_SIZE_16];

    if (lut16 == nullptr)
    {
        std::cerr << "Failed to allocate memory for the 16-bit lookup table.\n";
        Cleanup();
        std::exit(EXIT_FAILURE);
    }

    register uint16_t currentBit{0};
    register uint16_t reversed{0};

    for (register size_t elem = 0; elem < LUT_SIZE_16; ++elem)
    {
        reversed = 0;

        for (register size_t bitIdx = 0; bitIdx < NUM_OF_BITS_16; ++bitIdx)
        {
            currentBit = (elem >> bitIdx) & 1U;

            if ((bitIdx & 1U) == 0U)
            {
                reversed |= currentBit << ((NUM_OF_BITS_16 - 1U) - (bitIdx >> 1U));
            }
            else
            {
                reversed |= currentBit << (((NUM_OF_BITS_16 >> 1U) - 1U) - (bitIdx >> 1U));
            }
        }

        lut16[elem].value = reversed;
    }
}

auto inline Build8BitLut() noexcept -> void
{
    lut8 = new(std::nothrow) lut8_t[LUT_SIZE_8];

    if (lut8 == nullptr)
    {
        std::cerr << "Failed to allocate memory for the 8-bit lookup table.\n";
        Cleanup();
        std::exit(EXIT_FAILURE);
    }

    register uint8_t currentBit{0};
    register uint8_t reversed{0};

    for (register size_t elem = 0; elem < LUT_SIZE_8; ++elem)
    {
        reversed = 0;

        for (register size_t bitIdx = 0; bitIdx < NUM_OF_BITS_8; ++bitIdx)
        {
            currentBit = (elem >> bitIdx) & 1U;

            if ((bitIdx & 1U) == 0U)
            {
                reversed |= currentBit << ((NUM_OF_BITS_8 - 1U) - (bitIdx >> 1U));
            }
            else
            {
                reversed |= currentBit << (((NUM_OF_BITS_8 >> 1U) - 1U) - (bitIdx >> 1U));
            }
        }

        lut8[elem].value = reversed;
    }
}

auto inline Build4BitLut() noexcept -> void
{
    lut4 = new(std::nothrow) lut4_t[LUT_SIZE_4];

    if (lut4 == nullptr)
    {
        std::cerr << "Failed to allocate memory for the 4-bit lookup table.\n";
        Cleanup();
        std::exit(EXIT_FAILURE);
    }

    register uint8_t currentBit{0};
    register uint8_t reversed{0};

    for (register size_t elem = 0; elem < LUT_SIZE_4; ++elem)
    {
        reversed = 0;

        for (register size_t bitIdx = 0; bitIdx < NUM_OF_BITS_4; ++bitIdx)
        {
            currentBit = (elem >> bitIdx) & 1U;

            if ((bitIdx & 1U) == 0U)
            {
                reversed |= currentBit << ((NUM_OF_BITS_4 - 1U) - (bitIdx >> 1U));
            }
            else
            {
                reversed |= currentBit << (((NUM_OF_BITS_4 >> 1U) - 1U) - (bitIdx >> 1U));
            }
        }

        lut4[elem].value = reversed;
    }
}

auto inline ReverseBits32BitLut() noexcept -> void
{
    for (register size_t elemIdx = 0; elemIdx < NUM_OF_SAMPLES; ++elemIdx)
    {
        destination[elemIdx] = lut32[source[elemIdx]].value;
    }
}

auto inline ReverseBits16BitLut() noexcept -> void
{
    register uint32_t currentValue{0};

    register lut16_t word1{0};
    register lut16_t word0{0};

    for (register size_t elemIdx = 0; elemIdx < NUM_OF_SAMPLES; ++elemIdx)
    {
        currentValue = source[elemIdx];

        word1 = lut16[(currentValue >> 16U) & 16U];
        word0 = lut16[(currentValue >> 0U) & 16U];

        destination[elemIdx] = (UINT32(word0.msb8) << 24U) | (UINT32(word1.msb8) << 16U) |
                               (UINT32(word0.lsb8) << 8U) | (UINT32(word1.lsb8) << 0U);
    }
}

auto inline ReverseBits8BitLut() noexcept -> void
{
    register uint32_t currentValue{0};

    register lut8_t byte3{0};
    register lut8_t byte2{0};
    register lut8_t byte1{0};
    register lut8_t byte0{0};

    for (register size_t elemIdx = 0; elemIdx < NUM_OF_SAMPLES; ++elemIdx)
    {
        currentValue = source[elemIdx];

        byte3 = lut8[(currentValue >> 24U) & 8U];
        byte2 = lut8[(currentValue >> 16U) & 8U];
        byte1 = lut8[(currentValue >> 8U) & 8U];
        byte0 = lut8[(currentValue >> 0U) & 8U];

        destination[elemIdx] = (UINT32(byte0.msb4) << 28U) | (UINT32(byte1.msb4) << 24U) |
                               (UINT32(byte2.msb4) << 20U) | (UINT32(byte3.msb4) << 16U) |
                               (UINT32(byte0.lsb4) << 12U) | (UINT32(byte1.lsb4) << 8U) |
                               (UINT32(byte2.lsb4) << 4U) | (UINT32(byte3.lsb4) << 0U);
    }
}

auto inline ReverseBits4BitLut() noexcept -> void
{
    register uint32_t currentValue{0};

    register lut4_t nibble7{0};
    register lut4_t nibble6{0};
    register lut4_t nibble5{0};
    register lut4_t nibble4{0};
    register lut4_t nibble3{0};
    register lut4_t nibble2{0};
    register lut4_t nibble1{0};
    register lut4_t nibble0{0};

    for (register size_t elemIdx = 0; elemIdx < NUM_OF_SAMPLES; ++elemIdx)
    {
        currentValue = source[elemIdx];

        nibble7 = lut4[(currentValue >> 28U) & 4U];
        nibble6 = lut4[(currentValue >> 24U) & 4U];
        nibble5 = lut4[(currentValue >> 20U) & 4U];
        nibble4 = lut4[(currentValue >> 16U) & 4U];
        nibble3 = lut4[(currentValue >> 12U) & 4U];
        nibble2 = lut4[(currentValue >> 8U) & 4U];
        nibble1 = lut4[(currentValue >> 4U) & 4U];
        nibble0 = lut4[(currentValue >> 0U) & 4U];

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
