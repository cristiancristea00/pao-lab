#pragma once

#include <array>
#include <cstdint>
#include <string_view>


class DES
{
public:
    explicit DES(std::string_view const key) noexcept;

    [[nodiscard]] auto Encrypt(std::uint64_t const plaintext) const noexcept -> std::uint64_t;

    [[nodiscard]] auto Decrypt(std::uint64_t const ciphertext) const noexcept -> std::uint64_t;

private:
    static constexpr std::size_t NUM_ROUNDS{16};
    static constexpr std::size_t BLOCK_SIZE{64U};
    static constexpr std::size_t HALF_BLOCK_SIZE{BLOCK_SIZE / 2U};
    static constexpr std::size_t KEY_SIZE{56U};
    static constexpr std::size_t SUBKEY_SIZE{48U};

    std::array<std::uint64_t, NUM_ROUNDS> const roundKeys;

    static auto GetRoundKeys(std::string_view const stringKey) noexcept -> std::array<std::uint64_t, NUM_ROUNDS>;

    [[nodiscard]] auto EncryptDecrypt(std::uint64_t const input, bool const encrypt) const noexcept -> std::uint64_t;

    static auto ComputeInitialPermutation(std::uint64_t const input) noexcept -> std::uint64_t;

    static auto ComputeFinalPermutation(std::uint64_t const input) noexcept -> std::uint64_t;

    static auto ComputePermutation(std::uint64_t const input, std::uint8_t const * const permutation, std::size_t const size) noexcept -> std::uint64_t;

    [[nodiscard]] auto GetRoundKey(std::size_t const round, bool const encrypt) const noexcept -> std::uint64_t;

    static auto ComputeFeistel(std::uint32_t const input, std::uint64_t const key) noexcept -> std::uint32_t;

    static auto ComputeExpansion(std::uint32_t const input) noexcept -> std::uint64_t;

    static auto ComputeSBoxes(std::uint64_t const input) noexcept -> std::uint32_t;

    static auto ComputeFeistelPermutation(std::uint32_t const input) noexcept -> std::uint32_t;
};
