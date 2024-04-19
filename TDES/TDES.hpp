#pragma once

#include <string_view>
#include <vector>
#include <cstdint>

class TDES
{
public:
    static auto Encrypt(uint64_t const plaintext, std::string_view const key) noexcept -> uint64_t;
    static auto Decrypt(uint64_t const ciphertext, std::string_view const key) noexcept -> uint64_t;
    static auto EncryptFile(std::string_view const inputFileName, std::string_view const outputFileName, std::string_view const key) -> void;
    static auto DecryptFile(std::string_view const inputFileName, std::string_view const outputFileName, std::string_view const key) -> void;
    static auto GetRandomKey() noexcept -> std::string;
private:
    static auto GetKeyFromHex(std::string_view const key) -> uint64_t;
    static auto EncryptDecryptFile(std::string_view const inputFileName, std::string_view const outputFileName, std::string_view const key, bool const encrypt) -> void;
    static auto EncryptSequence(std::vector<uint64_t> const & input, std::string_view const key) noexcept -> std::vector<uint64_t>;
    static auto DecryptSequence(std::vector<uint64_t> const & input, std::string_view const key) noexcept -> std::vector<uint64_t>;
    static auto EncryptDecryptSequence(std::vector<uint64_t> const & input, std::string_view const key, bool const encrypt) -> std::vector<uint64_t>;
    static void CheckKey(std::string_view const key);
    static auto GetNonce() -> uint64_t;

    static constexpr std::size_t SEED = 0xDEADBEEF42UL;
    static constexpr std::size_t BYTES_IN_64BITS = 8U;
    static constexpr std::size_t KEY_SIZE = 168U;
    static constexpr std::size_t BYTE_SIZE = 8U;
    static constexpr std::size_t KEY_SIZE_IN_BYTES = KEY_SIZE / BYTE_SIZE;
    static constexpr std::size_t NO_OF_KEYS = 3U;
    static constexpr std::size_t LENGTH_RATIO = 2U;
};
