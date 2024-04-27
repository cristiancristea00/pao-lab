#include "DES.hpp"

#include <format>


DES::DES(std::string_view const key) noexcept: roundKeys(GetRoundKeys(key)) { }

auto DES::GetRoundKeys(std::string_view const stringKey) noexcept -> std::array<std::uint64_t, NUM_ROUNDS>
{
    static constexpr std::size_t SHIFT{28U};
    static constexpr std::size_t MASK{0x0FFF'FFFF};

    static constexpr std::array<std::uint8_t, KEY_SIZE> PERMUTED_CHOICE1{
        56, 48, 40, 32, 24, 16, 8,
        0, 57, 49, 41, 33, 25, 17,
        9, 1, 58, 50, 42, 34, 26,
        18, 10, 2, 59, 51, 43, 35,
        62, 54, 46, 38, 30, 22, 14,
        6, 61, 53, 45, 37, 29, 21,
        13, 5, 60, 52, 44, 36, 28,
        20, 12, 4, 27, 19, 11, 3,
    };

    static constexpr std::array<std::uint8_t, SUBKEY_SIZE> PERMUTED_CHOICE2{
        13, 16, 10, 23, 0, 4,
        2, 27, 14, 5, 20, 9,
        22, 18, 11, 3, 25, 7,
        15, 6, 26, 19, 12, 1,
        40, 51, 30, 36, 46, 54,
        29, 39, 50, 44, 32, 47,
        43, 48, 38, 55, 33, 52,
        45, 41, 49, 35, 28, 31,
    };

    static constexpr std::array<std::uint8_t, NUM_ROUNDS> LEFT_SHIFTS{
        1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1
    };

    std::array<std::uint64_t, NUM_ROUNDS> roundKeys{};

    std::uint64_t const key = std::stoull(std::string{stringKey}, nullptr, 16);
    std::uint64_t const permuted = ComputePermutation(key, PERMUTED_CHOICE1.data(), KEY_SIZE);
    std::uint64_t left = (permuted >> SHIFT) & MASK;
    std::uint64_t right = permuted & MASK;

    std::uint64_t combined{0};

    for (std::size_t round = 0; round < NUM_ROUNDS; ++round)
    {
        left = std::rotl(left, LEFT_SHIFTS[round]);
        right = std::rotl(right, LEFT_SHIFTS[round]);
        combined = (left << SHIFT) | right;
        roundKeys[round] = ComputePermutation(combined, PERMUTED_CHOICE2.data(), SUBKEY_SIZE);
    }

    return roundKeys;
}

auto DES::Encrypt(std::uint64_t const plaintext) const noexcept -> std::uint64_t
{
    return EncryptDecrypt(plaintext, true);
}

auto DES::Decrypt(std::uint64_t const ciphertext) const noexcept -> std::uint64_t
{
    return EncryptDecrypt(ciphertext, false);
}

auto DES::EncryptDecrypt(std::uint64_t const input, bool const encrypt) const noexcept -> std::uint64_t
{
    static constexpr std::size_t MASK{0xFFFF'FFFF};

    std::uint32_t left{0};
    std::uint32_t newLeft{0};
    std::uint32_t right{0};
    std::uint64_t output{0};

    auto const chunk = ComputeInitialPermutation(input);

    left = static_cast<std::uint32_t>(chunk >> HALF_BLOCK_SIZE);
    right = static_cast<std::uint32_t>(chunk & MASK);

    for (std::size_t round = 0; round < NUM_ROUNDS; ++round)
    {
        newLeft = right;
        right = left ^ ComputeFeistel(right, GetRoundKey(round, encrypt));
        left = newLeft;
    }

    output = (static_cast<std::uint64_t>(right) << HALF_BLOCK_SIZE) | static_cast<std::uint64_t>(left);

    return ComputeFinalPermutation(output);
}

auto DES::GetRoundKey(std::size_t const round, bool const encrypt) const noexcept -> std::uint64_t
{
    return roundKeys[encrypt ? round : NUM_ROUNDS - round - 1];
}

auto DES::ComputeInitialPermutation(std::uint64_t const input) noexcept -> std::uint64_t
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

    return ComputePermutation(input, INITIAL_PERMUTATION.data(), BLOCK_SIZE);
}

auto DES::ComputeFinalPermutation(std::uint64_t const input) noexcept -> std::uint64_t
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


    return ComputePermutation(input, FINAL_PERMUTATION.data(), BLOCK_SIZE);
}

auto DES::ComputePermutation(std::uint64_t const input, std::uint8_t const * const permutation, std::size_t const size) noexcept -> std::uint64_t
{
    std::uint64_t result{0};

    std::uint64_t currentBit{0};

    for (std::size_t bit = 0; bit < size; ++bit)
    {
        currentBit = (input >> permutation[bit]) & 0x01;
        result |= currentBit << bit;
    }

    return result;
}

auto DES::ComputeFeistel(std::uint32_t const input, std::uint64_t const key) noexcept -> std::uint32_t
{
    auto const expansion = ComputeExpansion(input);
    auto const xorWithKey = expansion ^ key;
    auto const substituted = ComputeSBoxes(xorWithKey);
    return ComputeFeistelPermutation(substituted);
}

auto DES::ComputeExpansion(std::uint32_t const input) noexcept -> std::uint64_t
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

    std::uint64_t result{0};

    std::uint64_t currentBit{0};

    for (std::size_t bit = 0; bit < SUBKEY_SIZE; ++bit)
    {
        currentBit = (input >> EXPANSION[bit]) & 0x01;
        result |= currentBit << bit;
    }

    return result;
}

auto DES::ComputeSBoxes(std::uint64_t const input) noexcept -> std::uint32_t
{
    static constexpr std::size_t SUB_BOX_INPUT_SIZE{6U};
    static constexpr std::size_t NUM_SUB_BOXES{8U};
    static constexpr std::size_t SBOX_ROWS{4U};
    static constexpr std::size_t SBOX_COLS{16U};
    static constexpr std::size_t SBOX_SHIFT{4U};
    static constexpr std::size_t SBOX0{SBOX_SHIFT * 0U};
    static constexpr std::size_t SBOX1{SBOX_SHIFT * 1U};
    static constexpr std::size_t SBOX2{SBOX_SHIFT * 2U};
    static constexpr std::size_t SBOX3{SBOX_SHIFT * 3U};
    static constexpr std::size_t SBOX4{SBOX_SHIFT * 4U};
    static constexpr std::size_t SBOX5{SBOX_SHIFT * 5U};
    static constexpr std::size_t SBOX6{SBOX_SHIFT * 6U};
    static constexpr std::size_t SBOX7{SBOX_SHIFT * 7U};

    static constexpr std::array<std::array<std::array<std::uint32_t, SBOX_COLS>, SBOX_ROWS>, NUM_SUB_BOXES> SUB_BOXES{
        {
            {
                {
                    {
                        {
                            14UL << SBOX0, 4UL << SBOX0, 13UL << SBOX0, 1UL << SBOX0, 2UL << SBOX0, 15UL << SBOX0, 11UL << SBOX0, 8UL << SBOX0,
                            3UL << SBOX0, 10UL << SBOX0, 6UL << SBOX0, 12UL << SBOX0, 5UL << SBOX0, 9UL << SBOX0, 0UL << SBOX0, 7UL << SBOX0
                        }
                    },
                    {
                        {
                            0UL << SBOX0, 15UL << SBOX0, 7UL << SBOX0, 4UL << SBOX0, 14UL << SBOX0, 2UL << SBOX0, 13UL << SBOX0, 1UL << SBOX0,
                            10UL << SBOX0, 6UL << SBOX0, 12UL << SBOX0, 11UL << SBOX0, 9UL << SBOX0, 5UL << SBOX0, 3UL << SBOX0, 8UL << SBOX0
                        }
                    },
                    {
                        {
                            4UL << SBOX0, 1UL << SBOX0, 14UL << SBOX0, 8UL << SBOX0, 13UL << SBOX0, 6UL << SBOX0, 2UL << SBOX0, 11UL << SBOX0,
                            15UL << SBOX0, 12UL << SBOX0, 9UL << SBOX0, 7UL << SBOX0, 3UL << SBOX0, 10UL << SBOX0, 5UL << SBOX0, 0UL << SBOX0
                        }
                    },
                    {
                        {
                            15UL << SBOX0, 12UL << SBOX0, 8UL << SBOX0, 2UL << SBOX0, 4UL << SBOX0, 9UL << SBOX0, 1UL << SBOX0, 7UL << SBOX0,
                            5UL << SBOX0, 11UL << SBOX0, 3UL << SBOX0, 14UL << SBOX0, 10UL << SBOX0, 0UL << SBOX0, 6UL << SBOX0, 13UL << SBOX0
                        }
                    },
                }
            },
            {
                {
                    {
                        {
                            15UL << SBOX1, 1UL << SBOX1, 8UL << SBOX1, 14UL << SBOX1, 6UL << SBOX1, 11UL << SBOX1, 3UL << SBOX1, 4UL << SBOX1,
                            9UL << SBOX1, 7UL << SBOX1, 2UL << SBOX1, 13UL << SBOX1, 12UL << SBOX1, 0UL << SBOX1, 5UL << SBOX1, 10UL << SBOX1
                        }
                    },
                    {
                        {
                            3UL << SBOX1, 13UL << SBOX1, 4UL << SBOX1, 7UL << SBOX1, 15UL << SBOX1, 2UL << SBOX1, 8UL << SBOX1, 14UL << SBOX1,
                            12UL << SBOX1, 0UL << SBOX1, 1UL << SBOX1, 10UL << SBOX1, 6UL << SBOX1, 9UL << SBOX1, 11UL << SBOX1, 5UL << SBOX1
                        }
                    },
                    {
                        {
                            0UL << SBOX1, 14UL << SBOX1, 7UL << SBOX1, 11UL << SBOX1, 10UL << SBOX1, 4UL << SBOX1, 13UL << SBOX1, 1UL << SBOX1,
                            5UL << SBOX1, 8UL << SBOX1, 12UL << SBOX1, 6UL << SBOX1, 9UL << SBOX1, 3UL << SBOX1, 2UL << SBOX1, 15UL << SBOX1
                        }
                    },
                    {
                        {
                            13UL << SBOX1, 8UL << SBOX1, 10UL << SBOX1, 1UL << SBOX1, 3UL << SBOX1, 15UL << SBOX1, 4UL << SBOX1, 2UL << SBOX1,
                            11UL << SBOX1, 6UL << SBOX1, 7UL << SBOX1, 12UL << SBOX1, 0UL << SBOX1, 5UL << SBOX1, 14UL << SBOX1, 9UL << SBOX1
                        }
                    },
                }
            },
            {
                {
                    {
                        {
                            10UL << SBOX2, 0UL << SBOX2, 9UL << SBOX2, 14UL << SBOX2, 6UL << SBOX2, 3UL << SBOX2, 15UL << SBOX2, 5UL << SBOX2,
                            1UL << SBOX2, 13UL << SBOX2, 12UL << SBOX2, 7UL << SBOX2, 11UL << SBOX2, 4UL << SBOX2, 2UL << SBOX2, 8UL << SBOX2
                        }
                    },
                    {
                        {
                            13UL << SBOX2, 7UL << SBOX2, 0UL << SBOX2, 9UL << SBOX2, 3UL << SBOX2, 4UL << SBOX2, 6UL << SBOX2, 10UL << SBOX2,
                            2UL << SBOX2, 8UL << SBOX2, 5UL << SBOX2, 14UL << SBOX2, 12UL << SBOX2, 11UL << SBOX2, 15UL << SBOX2, 1UL << SBOX2
                        }
                    },
                    {
                        {
                            13UL << SBOX2, 6UL << SBOX2, 4UL << SBOX2, 9UL << SBOX2, 8UL << SBOX2, 15UL << SBOX2, 3UL << SBOX2, 0UL << SBOX2,
                            11UL << SBOX2, 1UL << SBOX2, 2UL << SBOX2, 12UL << SBOX2, 5UL << SBOX2, 10UL << SBOX2, 14UL << SBOX2, 7UL << SBOX2
                        }
                    },
                    {
                        {
                            1UL << SBOX2, 10UL << SBOX2, 13UL << SBOX2, 0UL << SBOX2, 6UL << SBOX2, 9UL << SBOX2, 8UL << SBOX2, 7UL << SBOX2,
                            4UL << SBOX2, 15UL << SBOX2, 14UL << SBOX2, 3UL << SBOX2, 11UL << SBOX2, 5UL << SBOX2, 2UL << SBOX2, 12UL << SBOX2
                        }
                    },
                }
            },
            {
                {
                    {
                        {
                            7UL << SBOX3, 13UL << SBOX3, 14UL << SBOX3, 3UL << SBOX3, 0UL << SBOX3, 6UL << SBOX3, 9UL << SBOX3, 10UL << SBOX3,
                            1UL << SBOX3, 2UL << SBOX3, 8UL << SBOX3, 5UL << SBOX3, 11UL << SBOX3, 12UL << SBOX3, 4UL << SBOX3, 15UL << SBOX3
                        }
                    },
                    {
                        {
                            13UL << SBOX3, 8UL << SBOX3, 11UL << SBOX3, 5UL << SBOX3, 6UL << SBOX3, 15UL << SBOX3, 0UL << SBOX3, 3UL << SBOX3,
                            4UL << SBOX3, 7UL << SBOX3, 2UL << SBOX3, 12UL << SBOX3, 1UL << SBOX3, 10UL << SBOX3, 14UL << SBOX3, 9UL << SBOX3
                        }
                    },
                    {
                        {
                            10UL << SBOX3, 6UL << SBOX3, 9UL << SBOX3, 0UL << SBOX3, 12UL << SBOX3, 11UL << SBOX3, 7UL << SBOX3, 13UL << SBOX3,
                            15UL << SBOX3, 1UL << SBOX3, 3UL << SBOX3, 14UL << SBOX3, 5UL << SBOX3, 2UL << SBOX3, 8UL << SBOX3, 4UL << SBOX3
                        }
                    },
                    {
                        {
                            3UL << SBOX3, 15UL << SBOX3, 0UL << SBOX3, 6UL << SBOX3, 10UL << SBOX3, 1UL << SBOX3, 13UL << SBOX3, 8UL << SBOX3,
                            9UL << SBOX3, 4UL << SBOX3, 5UL << SBOX3, 11UL << SBOX3, 12UL << SBOX3, 7UL << SBOX3, 2UL << SBOX3, 14UL << SBOX3
                        }
                    },
                }
            },
            {
                {
                    {
                        {
                            2UL << SBOX4, 12UL << SBOX4, 4UL << SBOX4, 1UL << SBOX4, 7UL << SBOX4, 10UL << SBOX4, 11UL << SBOX4, 6UL << SBOX4,
                            8UL << SBOX4, 5UL << SBOX4, 3UL << SBOX4, 15UL << SBOX4, 13UL << SBOX4, 0UL << SBOX4, 14UL << SBOX4, 9UL << SBOX4
                        }
                    },
                    {
                        {
                            14UL << SBOX4, 11UL << SBOX4, 2UL << SBOX4, 12UL << SBOX4, 4UL << SBOX4, 7UL << SBOX4, 13UL << SBOX4, 1UL << SBOX4,
                            5UL << SBOX4, 0UL << SBOX4, 15UL << SBOX4, 10UL << SBOX4, 3UL << SBOX4, 9UL << SBOX4, 8UL << SBOX4, 6UL << SBOX4
                        }
                    },
                    {
                        {
                            4UL << SBOX4, 2UL << SBOX4, 1UL << SBOX4, 11UL << SBOX4, 10UL << SBOX4, 13UL << SBOX4, 7UL << SBOX4, 8UL << SBOX4,
                            15UL << SBOX4, 9UL << SBOX4, 12UL << SBOX4, 5UL << SBOX4, 6UL << SBOX4, 3UL << SBOX4, 0UL << SBOX4, 14UL << SBOX4
                        }
                    },
                    {
                        {
                            11UL << SBOX4, 8UL << SBOX4, 12UL << SBOX4, 7UL << SBOX4, 1UL << SBOX4, 14UL << SBOX4, 2UL << SBOX4, 13UL << SBOX4,
                            6UL << SBOX4, 15UL << SBOX4, 0UL << SBOX4, 9UL << SBOX4, 10UL << SBOX4, 4UL << SBOX4, 5UL << SBOX4, 3UL << SBOX4
                        }
                    },
                }
            },
            {
                {
                    {
                        {
                            12UL << SBOX5, 1UL << SBOX5, 10UL << SBOX5, 15UL << SBOX5, 9UL << SBOX5, 2UL << SBOX5, 6UL << SBOX5, 8UL << SBOX5,
                            0UL << SBOX5, 13UL << SBOX5, 3UL << SBOX5, 4UL << SBOX5, 14UL << SBOX5, 7UL << SBOX5, 5UL << SBOX5, 11UL << SBOX5
                        }
                    },
                    {
                        {
                            10UL << SBOX5, 15UL << SBOX5, 4UL << SBOX5, 2UL << SBOX5, 7UL << SBOX5, 12UL << SBOX5, 9UL << SBOX5, 5UL << SBOX5,
                            6UL << SBOX5, 1UL << SBOX5, 13UL << SBOX5, 14UL << SBOX5, 0UL << SBOX5, 11UL << SBOX5, 3UL << SBOX5, 8UL << SBOX5
                        }
                    },
                    {
                        {
                            9UL << SBOX5, 14UL << SBOX5, 15UL << SBOX5, 5UL << SBOX5, 2UL << SBOX5, 8UL << SBOX5, 12UL << SBOX5, 3UL << SBOX5,
                            7UL << SBOX5, 0UL << SBOX5, 4UL << SBOX5, 10UL << SBOX5, 1UL << SBOX5, 13UL << SBOX5, 11UL << SBOX5, 6UL << SBOX5
                        }
                    },
                    {
                        {
                            4UL << SBOX5, 3UL << SBOX5, 2UL << SBOX5, 12UL << SBOX5, 9UL << SBOX5, 5UL << SBOX5, 15UL << SBOX5, 10UL << SBOX5,
                            11UL << SBOX5, 14UL << SBOX5, 1UL << SBOX5, 7UL << SBOX5, 6UL << SBOX5, 0UL << SBOX5, 8UL << SBOX5, 13UL << SBOX5
                        }
                    },
                }
            },
            {
                {
                    {
                        {
                            4UL << SBOX6, 11UL << SBOX6, 2UL << SBOX6, 14UL << SBOX6, 15UL << SBOX6, 0UL << SBOX6, 8UL << SBOX6, 13UL << SBOX6,
                            3UL << SBOX6, 12UL << SBOX6, 9UL << SBOX6, 7UL << SBOX6, 5UL << SBOX6, 10UL << SBOX6, 6UL << SBOX6, 1UL << SBOX6
                        }
                    },
                    {
                        {
                            13UL << SBOX6, 0UL << SBOX6, 11UL << SBOX6, 7UL << SBOX6, 4UL << SBOX6, 9UL << SBOX6, 1UL << SBOX6, 10UL << SBOX6,
                            14UL << SBOX6, 3UL << SBOX6, 5UL << SBOX6, 12UL << SBOX6, 2UL << SBOX6, 15UL << SBOX6, 8UL << SBOX6, 6UL << SBOX6
                        }
                    },
                    {
                        {
                            1UL << SBOX6, 4UL << SBOX6, 11UL << SBOX6, 13UL << SBOX6, 12UL << SBOX6, 3UL << SBOX6, 7UL << SBOX6, 14UL << SBOX6,
                            10UL << SBOX6, 15UL << SBOX6, 6UL << SBOX6, 8UL << SBOX6, 0UL << SBOX6, 5UL << SBOX6, 9UL << SBOX6, 2UL << SBOX6
                        }
                    },
                    {
                        {
                            6UL << SBOX6, 11UL << SBOX6, 13UL << SBOX6, 8UL << SBOX6, 1UL << SBOX6, 4UL << SBOX6, 10UL << SBOX6, 7UL << SBOX6,
                            9UL << SBOX6, 5UL << SBOX6, 0UL << SBOX6, 15UL << SBOX6, 14UL << SBOX6, 2UL << SBOX6, 3UL << SBOX6, 12UL << SBOX6
                        }
                    },
                }
            },
            {
                {
                    {
                        {
                            13UL << SBOX7, 2UL << SBOX7, 8UL << SBOX7, 4UL << SBOX7, 6UL << SBOX7, 15UL << SBOX7, 11UL << SBOX7, 1UL << SBOX7,
                            10UL << SBOX7, 9UL << SBOX7, 3UL << SBOX7, 14UL << SBOX7, 5UL << SBOX7, 0UL << SBOX7, 12UL << SBOX7, 7UL << SBOX7
                        }
                    },
                    {
                        {
                            1UL << SBOX7, 15UL << SBOX7, 13UL << SBOX7, 8UL << SBOX7, 10UL << SBOX7, 3UL << SBOX7, 7UL << SBOX7, 4UL << SBOX7,
                            12UL << SBOX7, 5UL << SBOX7, 6UL << SBOX7, 11UL << SBOX7, 0UL << SBOX7, 14UL << SBOX7, 9UL << SBOX7, 2UL << SBOX7
                        }
                    },
                    {
                        {
                            7UL << SBOX7, 11UL << SBOX7, 4UL << SBOX7, 1UL << SBOX7, 9UL << SBOX7, 12UL << SBOX7, 14UL << SBOX7, 2UL << SBOX7,
                            0UL << SBOX7, 6UL << SBOX7, 10UL << SBOX7, 13UL << SBOX7, 15UL << SBOX7, 3UL << SBOX7, 5UL << SBOX7, 8UL << SBOX7
                        }
                    },
                    {
                        {
                            2UL << SBOX7, 1UL << SBOX7, 14UL << SBOX7, 7UL << SBOX7, 4UL << SBOX7, 10UL << SBOX7, 8UL << SBOX7, 13UL << SBOX7,
                            15UL << SBOX7, 12UL << SBOX7, 9UL << SBOX7, 0UL << SBOX7, 3UL << SBOX7, 5UL << SBOX7, 6UL << SBOX7, 11UL << SBOX7
                        }
                    },
                }
            },
        }
    };

    std::uint32_t result{0};

    std::uint64_t chunk{0};
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

auto DES::ComputeFeistelPermutation(std::uint32_t const input) noexcept -> std::uint32_t
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

    std::uint32_t result{0};

    std::uint32_t currentBit{0};

    for (std::size_t bit = 0; bit < HALF_BLOCK_SIZE; ++bit)
    {
        currentBit = (input >> FEISTEL_PERMUTATION[bit]) & 0x01;
        result |= currentBit << bit;
    }

    return result;
}
