#include <format>
#include "DES.hpp"

auto DES::Encrypt(uint64_t const plaintext, std::string_view const key) noexcept -> uint64_t
{
    return plaintext;
}

auto DES::Decrypt(uint64_t const ciphertext, std::string_view const key) noexcept -> uint64_t
{
    return ciphertext;
}

void DES::CheckKey(std::string_view const key)
{
    if (key.size() / 2 != KEY_SIZE / BYTE_SIZE)
    {
        throw std::invalid_argument{std::format("Invalid key size (expected: {}, actual: {})", KEY_SIZE / BYTE_SIZE, key.size() / 2)};
    }
}
