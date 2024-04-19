#pragma once

#include <cstdint>
#include <string_view>

class DES
{
public:
    static auto Encrypt(uint64_t const plaintext, uint64_t const key) noexcept -> uint64_t;
    static auto Decrypt(uint64_t const ciphertext, uint64_t const key) noexcept -> uint64_t;
private:
    static auto EncryptDecrypt(uint64_t const input, uint64_t const key, bool const encrypt) noexcept -> uint64_t;

    static constexpr std::size_t KEY_SIZE = 56U;
    static constexpr std::size_t BYTE_SIZE = 8U;
    static constexpr std::size_t KEY_SIZE_IN_BYTES = KEY_SIZE / BYTE_SIZE;
    static constexpr std::size_t LENGTH_RATIO = 2U;

    static constexpr std::array<std::uint8_t, 64> INITIAL_PERMUTATION{
        57, 49, 41, 33, 25, 17, 9, 1,
        59, 51, 43, 35, 27, 19, 11, 3,
        61, 53, 45, 37, 29, 21, 13, 5,
        63, 55, 47, 39, 31, 23, 15, 7,
        56, 48, 40, 32, 24, 16, 8, 0,
        58, 50, 42, 34, 26, 18, 10, 2,
        60, 52, 44, 36, 28, 20, 12, 4,
        62, 54, 46, 38, 30, 22, 14, 6
    };

    static constexpr std::array<std::uint8_t, 64> FINAL_PERMUTATION{
        39, 7, 47, 15, 55, 23, 63, 31,
        38, 6, 46, 14, 54, 22, 62, 30,
        37, 5, 45, 13, 53, 21, 61, 29,
        36, 4, 44, 12, 52, 20, 60, 28,
        35, 3, 43, 11, 51, 19, 59, 27,
        34, 2, 42, 10, 50, 18, 58, 26,
        33, 1, 41, 9, 49, 17, 57, 25,
        32, 0, 40, 8, 48, 16, 56, 24
    };

    static constexpr std::array<std::uint8_t, 48> EXPANSION{
        31, 0, 1, 2, 3, 4,
        3, 4, 5, 6, 7, 8,
        7, 8, 9, 10, 11, 12,
        11, 12, 13, 14, 15, 16,
        15, 16, 17, 18, 19, 20,
        19, 20, 21, 22, 23, 24,
        23, 24, 25, 26, 27, 28,
        27, 28, 29, 30, 31, 0
    };

    static constexpr std::array<std::uint8_t, 32> PERMUTATION{
        15, 6, 19, 20, 28, 11, 27, 16,
        0, 14, 22, 25, 4, 17, 30, 9,
        1, 7, 23, 13, 31, 26, 2, 8,
        18, 12, 29, 5, 21, 10, 3, 24
    };

    static constexpr std::array<std::uint8_t, 56> PERMUTED_CHOICE1{
        56, 48, 40, 32, 24, 16, 8,
        0, 57, 49, 41, 33, 25, 17,
        9, 1, 58, 50, 42, 34, 26,
        18, 10, 2, 59, 51, 43, 35,
        62, 54, 46, 38, 30, 22, 14,
        6, 61, 53, 45, 37, 29, 21,
        13, 5, 60, 52, 44, 36, 28,
        20, 12, 4, 27, 19, 11, 3,
    };

    static constexpr std::array<std::uint8_t, 48> PERMUTED_CHOICE2{
        13, 16, 10, 23, 0, 4,
        2, 27, 14, 5, 20, 9,
        22, 18, 11, 3, 25, 7,
        15, 6, 26, 19, 12, 1,
        40, 51, 30, 36, 46, 54,
        29, 39, 50, 44, 32, 47,
        43, 48, 38, 55, 33, 52,
        45, 41, 49, 35, 28, 31,
    };
};
