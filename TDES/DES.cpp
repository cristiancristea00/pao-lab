#include <format>
#include <iostream>
#include "DES.hpp"

auto DES::Encrypt(uint64_t const plaintext, uint64_t const key) noexcept -> uint64_t
{
    return EncryptDecrypt(plaintext, key, true);
}

auto DES::Decrypt(uint64_t const ciphertext, uint64_t const key) noexcept -> uint64_t
{
    return EncryptDecrypt(ciphertext, key, false);
}

auto DES::EncryptDecrypt(uint64_t const input, uint64_t const key, bool const encrypt) noexcept -> uint64_t
{
    return 0;
}
