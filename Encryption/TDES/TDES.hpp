#pragma once

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include "DES.hpp"

class TDES
{
public:
    explicit TDES(std::string_view const stringKey) noexcept;

    [[nodiscard]] auto Encrypt(std::uint64_t const plaintext) const noexcept -> std::uint64_t;

    [[nodiscard]] auto Decrypt(std::uint64_t const ciphertext) const noexcept -> std::uint64_t;

    [[nodiscard]] auto EncryptFile(std::string_view const inputFileName, std::string_view const outputFileName) const -> std::uint8_t;

    auto DecryptFile(std::string_view const inputFileName, std::string_view const outputFileName, std::uint8_t const lastBytes) const -> void;

    static auto GetRandomKey() noexcept -> std::string;

private:
    static constexpr std::size_t SEED{0xDEADBEEF42UL};
    static constexpr std::size_t BYTES_IN_64BITS{8U};
    static constexpr std::size_t KEY_SIZE{168U};
    static constexpr std::size_t BYTE_SIZE{8U};
    static constexpr std::size_t KEY_SIZE_IN_BYTES{KEY_SIZE / BYTE_SIZE};
    static constexpr std::size_t NUM_KEYS{3U};
    static constexpr std::size_t LENGTH_RATIO{2U};

    std::unique_ptr<DES> const des1;
    std::unique_ptr<DES> const des2;
    std::unique_ptr<DES> const des3;

    auto EncryptDecryptFile(std::string_view const inputFileName, std::string_view const outputFileName, bool const encrypt, std::uint8_t const lastBytes = 0) const -> std::uint8_t;

    [[nodiscard]] auto EncryptSequence(std::vector<std::uint64_t> const & input) const noexcept -> std::vector<std::uint64_t>;

    [[nodiscard]] auto DecryptSequence(std::vector<std::uint64_t> const & input) const noexcept -> std::vector<std::uint64_t>;

    [[nodiscard]] auto EncryptDecryptSequence(std::vector<std::uint64_t> const & input) const -> std::vector<std::uint64_t>;

    static auto GetNonce() -> std::uint64_t;
};
