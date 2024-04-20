#pragma once

#include <string_view>
#include <vector>
#include <cstdint>
#include <memory>

#include "DES.hpp"

class TDES
{
public:
    explicit TDES(std::string_view const key) noexcept;

    auto Encrypt(uint64_t const plaintext) const noexcept -> uint64_t;
    auto Decrypt(uint64_t const ciphertext) const noexcept -> uint64_t;
    auto EncryptFile(std::string_view const inputFileName, std::string_view const outputFileName) const -> void;
    auto DecryptFile(std::string_view const inputFileName, std::string_view const outputFileName) const -> void;
    static auto GetRandomKey() noexcept -> std::string;
private:
    static constexpr std::size_t SEED{0xDEADBEEF42UL};
    static constexpr std::size_t BYTES_IN_64BITS{8U};
    static constexpr std::size_t KEY_SIZE{168U};
    static constexpr std::size_t BYTE_SIZE{8U};
    static constexpr std::size_t KEY_SIZE_IN_BYTES{KEY_SIZE / BYTE_SIZE};
    static constexpr std::size_t NUM_KEYS{3U};
    static constexpr std::size_t LENGTH_RATIO{2U};

    std::string_view const key;
    std::unique_ptr<DES> const des1;
    std::unique_ptr<DES> const des2;
    std::unique_ptr<DES> const des3;

    auto EncryptDecryptFile(std::string_view const inputFileName, std::string_view const outputFileName, bool const encrypt)  const-> void;
    auto EncryptSequence(std::vector<uint64_t> const & input)const noexcept -> std::vector<uint64_t>;
    auto DecryptSequence(std::vector<uint64_t> const & input)const noexcept -> std::vector<uint64_t>;
    auto EncryptDecryptSequence(std::vector<uint64_t> const & input, bool const encrypt) const -> std::vector<uint64_t>;
    static auto CheckKey(std::string_view const key) -> std::string_view;
    static auto GetNonce() -> uint64_t;
};
