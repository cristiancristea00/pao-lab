#pragma once

#include <cstdint>
#include <format>
#include <memory>
#include <string_view>
#include <vector>
#include <wmmintrin.h>


using int128_t = __m128i;

using BufferType = std::uint8_t;
using BufferPointer = std::unique_ptr<BufferType[]>;

enum class KeyType : std::uint8_t
{
    AES128 = 16,
    AES192 = 24,
    AES256 = 32,
};

enum class KeyLenSize : std::uint8_t;

enum class RoundSize : std::uint8_t;


template <typename Enumeration>
constexpr auto as_num(Enumeration const value) noexcept -> std::size_t requires std::is_enum_v<Enumeration>
{
    return static_cast<std::size_t>(value);
}

class AES
{
public:
    explicit AES(std::string_view const stringKey);

    auto Encrypt(BufferType * const plaintext) const noexcept -> BufferPointer;

    auto Decrypt(BufferType * const ciphertext) const noexcept -> BufferPointer;

    auto EncryptFile(std::string_view const inputFileName, std::string_view const outputFileName) const -> void;

    auto DecryptFile(std::string_view const inputFileName, std::string_view const outputFileName) const -> void;

    static auto GetRandomKey(KeyType const keyType = KeyType::AES128) noexcept -> std::string;

private:
    static constexpr std::size_t SEED{0xDEADBEEF42UL};

    std::size_t const keySize;
    std::size_t const rounds;
    BufferPointer const roundKeys;

    static auto GetKeySize(std::string_view const stringKey) -> std::size_t;

    static auto GetNumRounds(std::string_view const stringKey) -> std::size_t;

    static auto GetRoundKeys(std::string_view const stringKey) -> BufferPointer;

    static auto GetKeyFromString(std::string_view const stringKey) noexcept -> BufferPointer;

    static auto KeyExpansion128(std::string_view const stringKey) noexcept -> BufferPointer;

    static auto KeyExpansionAssist128(int128_t temp1, int128_t temp2) noexcept -> int128_t;

    static auto KeyExpansion192(std::string_view const stringKey) noexcept -> BufferPointer;

    static auto KeyExpansion256(std::string_view const stringKey) noexcept -> BufferPointer;
};
