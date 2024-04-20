#include <format>
#include <iostream>
#include "DES.hpp"

DES::DES(std::string_view const key) noexcept: roundKeys(GetRoundKeys(key)) { }

auto DES::GetRoundKeys(std::string_view const key) noexcept -> std::array<std::uint64_t, NUM_ROUNDS>
{
    // TODO: Implement this function
    return {};
}

auto DES::Encrypt(uint64_t const plaintext) const noexcept -> uint64_t
{
    return EncryptDecrypt(plaintext, true);
}

auto DES::Decrypt(uint64_t const ciphertext) const noexcept -> uint64_t
{
    return EncryptDecrypt(ciphertext, false);
}

auto DES::EncryptDecrypt(uint64_t const input, bool const encrypt) noexcept -> uint64_t
{
    uint32_t left{0};
    uint32_t newLeft{0};
    uint32_t right{0};
    uint64_t roundKey{0};
    uint64_t output{0};

    auto const initialPermutation = ComputeInitialPermutation(input);

    left = static_cast<uint32_t>(initialPermutation >> HALF_BLOCK_SIZE);
    right = static_cast<uint32_t>(initialPermutation & 0xFFFF'FFFFULL);

    for (std::size_t round = 0; round < NUM_ROUNDS; ++round)
    {
        roundKey = GetRoundKey(round, encrypt);

        newLeft = right;
        right = left ^ ComputeFeistel(right, roundKey);
        left = newLeft;
    }

    output = (static_cast<uint64_t>(right) << HALF_BLOCK_SIZE) | static_cast<uint64_t>(left);

    return ComputeFinalPermutation(output);
}

auto DES::GetRoundKey(std::size_t const round, bool const encrypt) noexcept -> uint64_t
{
    // TODO: Implement this function
    return 0;
}

auto DES::ComputeInitialPermutation(uint64_t const input) noexcept -> uint64_t
{
    static constexpr std::array<std::uint8_t, BLOCK_SIZE> INITIAL_PERMUTATION{
        57, 49, 41, 33, 25, 17, 9, 1,
        59, 51, 43, 35, 27, 19, 11, 3,
        61, 53, 45, 37, 29, 21, 13, 5,
        63, 55, 47, 39, 31, 23, 15, 7,
        56, 48, 40, 32, 24, 16, 8, 0,
        58, 50, 42, 34, 26, 18, 10, 2,
        60, 52, 44, 36, 28, 20, 12, 4,
        62, 54, 46, 38, 30, 22, 14, 6
    };

    return ComputePermutation(input, INITIAL_PERMUTATION);
}

auto DES::ComputeFinalPermutation(uint64_t const input) noexcept -> uint64_t
{
    static constexpr std::array<std::uint8_t, BLOCK_SIZE> FINAL_PERMUTATION{
        39, 7, 47, 15, 55, 23, 63, 31,
        38, 6, 46, 14, 54, 22, 62, 30,
        37, 5, 45, 13, 53, 21, 61, 29,
        36, 4, 44, 12, 52, 20, 60, 28,
        35, 3, 43, 11, 51, 19, 59, 27,
        34, 2, 42, 10, 50, 18, 58, 26,
        33, 1, 41, 9, 49, 17, 57, 25,
        32, 0, 40, 8, 48, 16, 56, 24
    };


    return ComputePermutation(input, FINAL_PERMUTATION);
}

auto DES::ComputePermutation(uint64_t const input, std::array<std::uint8_t, BLOCK_SIZE> const & permutation) noexcept -> uint64_t
{
    uint64_t result{0};

    uint64_t currentBit{0};

    for (std::size_t bit = 0; bit < BLOCK_SIZE; ++bit)
    {
        currentBit = (input >> permutation[bit]) & 0x01;
        result |= currentBit << bit;
    }

    return result;
}

auto DES::ComputeFeistel(uint32_t const input, uint64_t const key) noexcept -> uint32_t
{
    auto const expansion = ComputeExpansion(input);
    auto const xorWithKey = expansion ^ key;
    auto const substituted = ComputeSBoxes(xorWithKey);
    return ComputeFeistelPermutation(substituted);
}

auto DES::ComputeExpansion(uint32_t const input) noexcept -> uint64_t
{
    static constexpr std::array<std::uint8_t, SUBKEY_SIZE> EXPANSION{
        31, 0, 1, 2, 3, 4,
        3, 4, 5, 6, 7, 8,
        7, 8, 9, 10, 11, 12,
        11, 12, 13, 14, 15, 16,
        15, 16, 17, 18, 19, 20,
        19, 20, 21, 22, 23, 24,
        23, 24, 25, 26, 27, 28,
        27, 28, 29, 30, 31, 0
    };

    uint64_t result{0};

    uint64_t currentBit{0};

    for (std::size_t bit = 0; bit < SUBKEY_SIZE; ++bit)
    {
        currentBit = (input >> EXPANSION[bit]) & 0x01;
        result |= currentBit << bit;
    }

    return result;
}

auto DES::ComputeSBoxes(uint64_t const input) noexcept -> uint32_t
{
    uint32_t result{0};

    uint64_t chunk{0};
    std::size_t subBoxIndex{0};
    std::size_t row{0};
    std::size_t col{0};

    for (std::size_t idx = 0; idx < NUM_SUB_BOXES; ++idx)
    {
        chunk = input >> (idx * SUB_BOX_INPUT_SIZE);
        subBoxIndex = NUM_SUB_BOXES - idx - 1;

        row = ((chunk >> 4) & 0x02) | (chunk & 0x01);
        col = (chunk & 0x1E) >> 1;

        result |= SUB_BOXES[subBoxIndex][row][col];
    }

    return result;
}

auto DES::ComputeFeistelPermutation(uint32_t const input) noexcept -> uint32_t
{
    static constexpr std::array<std::uint8_t, HALF_BLOCK_SIZE> FEISTEL_PERMUTATION{
        15, 6, 19, 20,
        28, 11, 27, 16,
        0, 14, 22, 25,
        4, 17, 30, 9,
        1, 7, 23, 13,
        31, 26, 2, 8,
        18, 12, 29, 5,
        21, 10, 3, 24
    };

    uint32_t result{0};

    uint32_t currentBit{0};

    for (std::size_t bit = 0; bit < HALF_BLOCK_SIZE; ++bit)
    {
        currentBit = (input >> FEISTEL_PERMUTATION[bit]) & 0x01;
        result |= currentBit << bit;
    }

    return result;
}
