#pragma once

#include <cstdint>
#include <string_view>

class DES
{
public:
    static auto Encrypt(uint64_t const plaintext, std::string_view const key) noexcept -> uint64_t;
    static auto Decrypt(uint64_t const ciphertext, std::string_view const key) noexcept -> uint64_t;
private:
    static void CheckKey(std::string_view const key);

    static constexpr std::size_t KEY_SIZE = 56U;
    static constexpr std::size_t BYTE_SIZE = 8U;
    static constexpr std::size_t KEY_SIZE_IN_BYTES = KEY_SIZE / BYTE_SIZE;
};
