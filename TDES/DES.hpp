#pragma once

#include <cstdint>
#include <array>
#include <string_view>

class DES
{
public:
    explicit DES(std::string_view const key) noexcept;

    [[nodiscard]] auto Encrypt(uint64_t const plaintext) const noexcept -> uint64_t;
    [[nodiscard]] auto Decrypt(uint64_t const ciphertext) const noexcept -> uint64_t;
private:
    static constexpr std::size_t NUM_ROUNDS{16};
    static constexpr std::size_t BLOCK_SIZE{64U};
    static constexpr std::size_t HALF_BLOCK_SIZE{BLOCK_SIZE / 2U};
    static constexpr std::size_t KEY_SIZE{56U};
    static constexpr std::size_t SUBKEY_SIZE{48U};

    std::array<std::uint64_t, NUM_ROUNDS> const roundKeys;

    static auto GetRoundKeys(std::string_view const stringKey) noexcept -> std::array<std::uint64_t, NUM_ROUNDS>;
    [[nodiscard]] auto EncryptDecrypt(uint64_t const input, bool const encrypt) const noexcept -> uint64_t;
    static auto ComputeInitialPermutation(uint64_t const input) noexcept -> uint64_t;
    static auto ComputeFinalPermutation(uint64_t const input) noexcept -> uint64_t;
    static auto ComputePermutation(uint64_t const input, uint8_t const * const permutation, std::size_t const size) noexcept -> uint64_t;
    [[nodiscard]] auto GetRoundKey(std::size_t const round, bool const encrypt) const noexcept -> uint64_t;
    static auto ComputeFeistel(uint32_t const input, uint64_t const key) noexcept -> uint32_t;
    static auto ComputeExpansion(uint32_t const input) noexcept -> uint64_t;
    static auto ComputeSBoxes(uint64_t const input) noexcept -> uint32_t;
    static auto ComputeFeistelPermutation(uint32_t const input) noexcept -> uint32_t;
};
