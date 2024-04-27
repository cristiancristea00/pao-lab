#include "AES.hpp"

#include <format>
#include <random>
#include <string>


#define INT128_PTR(x)    reinterpret_cast<int128_t *>(x)


enum class KeyLenSize : std::uint8_t
{
    AES128 = 32,
    AES192 = 48,
    AES256 = 64,
};

enum class RoundSize : std::uint8_t
{
    AES128 = 11,
    AES192 = 13,
    AES256 = 15,
};


AES::AES(std::string_view const stringKey) : keySize{GetKeySize(stringKey)},
                                             rounds{GetNumRounds(stringKey)},
                                             roundKeys{GetRoundKeys(stringKey)} { }

auto AES::Encrypt(BufferType * const plaintext) const noexcept -> BufferPointer
{
    auto result = std::make_unique<BufferType[]>(keySize);

    int128_t temp;

    temp = _mm_loadu_si128(INT128_PTR(plaintext));
    temp = _mm_xor_si128(temp, INT128_PTR(roundKeys.get())[0]);
    for (std::size_t round = 1; round < rounds; ++round)
    {
        temp = _mm_aesenc_si128(temp, INT128_PTR(roundKeys.get())[round]);
    }
    temp = _mm_aesenclast_si128(temp, INT128_PTR(roundKeys.get())[rounds]);
    _mm_storeu_si128(INT128_PTR(result.get()), temp);

    return result;
}

auto AES::Decrypt(BufferType * const ciphertext) const noexcept -> BufferPointer
{
    auto result = std::make_unique<BufferType[]>(keySize);

    int128_t temp;

    temp = _mm_loadu_si128(INT128_PTR(ciphertext));
    temp = _mm_xor_si128(temp, INT128_PTR(roundKeys.get())[rounds]);
    for (std::size_t round = 1; round < rounds; ++round)
    {
        temp = _mm_aesdec_si128(temp, INT128_PTR(roundKeys.get())[rounds + round]);
    }
    temp = _mm_aesdeclast_si128(temp, INT128_PTR(roundKeys.get())[0]);
    _mm_storeu_si128(INT128_PTR(result.get()), temp);

    return result;
}

auto AES::EncryptFile(std::string_view const inputFileName, std::string_view const outputFileName) const -> void
{
    // TODO: Implement this function
}

auto AES::DecryptFile(std::string_view const inputFileName, std::string_view const outputFileName) const -> void
{
    // TODO: Implement this function
}

auto AES::GetRandomKey(KeyType const keyType) noexcept -> std::string
{
    std::mt19937_64 randomEngine{SEED};
    std::uniform_int_distribution<std::uint8_t> randomDistribution{0, std::numeric_limits<std::uint8_t>::max()};

    std::string key;
    key.reserve(as_num(keyType));

    for (std::size_t idx = 0; idx < as_num(keyType); ++idx)
    {
        key.append(std::format("{:02X}", randomDistribution(randomEngine)));
    }

    return key;
}

auto AES::GetKeySize(std::string_view const stringKey) -> std::size_t
{
    switch (stringKey.size())
    {
        case as_num(KeyLenSize::AES128) :
            return as_num(KeyType::AES128);
        case as_num(KeyLenSize::AES192) :
            return as_num(KeyType::AES192);
        case as_num(KeyLenSize::AES256) :
            return as_num(KeyType::AES256);
        default :
            throw std::invalid_argument{std::format("Invalid key size {}", stringKey.size())};
    }
}

auto AES::GetNumRounds(std::string_view const stringKey) -> std::size_t
{
    switch (stringKey.size())
    {
        case as_num(KeyLenSize::AES128) :
            return as_num(RoundSize::AES128) - 1;
        case as_num(KeyLenSize::AES192) :
            return as_num(RoundSize::AES192) - 1;
        case as_num(KeyLenSize::AES256) :
            return as_num(RoundSize::AES256) - 1;
        default :
            throw std::invalid_argument{std::format("Invalid key size {}", stringKey.size())};
    }
}

auto AES::GetRoundKeys(std::string_view const stringKey) -> BufferPointer
{
    switch (stringKey.size())
    {
        case as_num(KeyLenSize::AES128) :
            return KeyExpansion128(stringKey);
        case as_num(KeyLenSize::AES192) :
            return KeyExpansion192(stringKey);
        case as_num(KeyLenSize::AES256) :
            return KeyExpansion256(stringKey);
        default :
            throw std::invalid_argument{std::format("Invalid key size {}", stringKey.size())};
    }
}

auto AES::GetKeyFromString(std::string_view const stringKey) noexcept -> BufferPointer
{
    auto key = std::make_unique<BufferType[]>(stringKey.size() / 2);

    for (std::size_t idx = 0; idx < stringKey.size(); idx += 2)
    {
        key[idx / 2] = std::stoul(std::string{stringKey.substr(idx, 2)}, nullptr, 16);
    }

    return key;
}

auto AES::KeyExpansion128(std::string_view const stringKey) noexcept -> BufferPointer
{
    auto const key = GetKeyFromString(stringKey);

    auto constexpr totalSize = (as_num(RoundSize::AES128) + as_num(RoundSize::AES128) - 2) * as_num(KeyType::AES128);
    auto roundKeys = std::make_unique<std::uint8_t[]>(totalSize);

    int128_t temp1;
    int128_t temp2;

    auto * const keySchedule = INT128_PTR(roundKeys.get());

    temp1 = _mm_loadu_si128(INT128_PTR(key.get()));
    keySchedule[0] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x01);
    temp1 = KeyExpansionAssist128(temp1, temp2);
    keySchedule[1] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x02);
    temp1 = KeyExpansionAssist128(temp1, temp2);
    keySchedule[2] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x04);
    temp1 = KeyExpansionAssist128(temp1, temp2);
    keySchedule[3] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x08);
    temp1 = KeyExpansionAssist128(temp1, temp2);
    keySchedule[4] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x10);
    temp1 = KeyExpansionAssist128(temp1, temp2);
    keySchedule[5] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x20);
    temp1 = KeyExpansionAssist128(temp1, temp2);
    keySchedule[6] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x40);
    temp1 = KeyExpansionAssist128(temp1, temp2);
    keySchedule[7] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x80);
    temp1 = KeyExpansionAssist128(temp1, temp2);
    keySchedule[8] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x1B);
    temp1 = KeyExpansionAssist128(temp1, temp2);
    keySchedule[9] = temp1;
    temp2 = _mm_aeskeygenassist_si128(temp1, 0x36);
    temp1 = KeyExpansionAssist128(temp1, temp2);
    keySchedule[10] = temp1;

    keySchedule[11] = _mm_aesimc_si128(keySchedule[9]);
    keySchedule[12] = _mm_aesimc_si128(keySchedule[8]);
    keySchedule[13] = _mm_aesimc_si128(keySchedule[7]);
    keySchedule[14] = _mm_aesimc_si128(keySchedule[6]);
    keySchedule[15] = _mm_aesimc_si128(keySchedule[5]);
    keySchedule[16] = _mm_aesimc_si128(keySchedule[4]);
    keySchedule[17] = _mm_aesimc_si128(keySchedule[3]);
    keySchedule[18] = _mm_aesimc_si128(keySchedule[2]);
    keySchedule[19] = _mm_aesimc_si128(keySchedule[1]);

    return roundKeys;
}

auto AES::KeyExpansionAssist128(int128_t temp1, int128_t temp2) noexcept -> int128_t
{
    int128_t temp3;

    temp2 = _mm_shuffle_epi32(temp2, 0xFF);
    temp3 = _mm_slli_si128(temp1, 0x04);
    temp1 = _mm_xor_si128(temp1, temp3);
    temp3 = _mm_slli_si128(temp3, 0x04);
    temp1 = _mm_xor_si128(temp1, temp3);
    temp3 = _mm_slli_si128(temp3, 0x04);
    temp1 = _mm_xor_si128(temp1, temp3);
    temp1 = _mm_xor_si128(temp1, temp2);
    return temp1;
}

auto AES::KeyExpansion192(std::string_view const stringKey) noexcept -> BufferPointer
{
    // TODO: Implement this function
    return nullptr;
}

auto AES::KeyExpansion256(std::string_view const stringKey) noexcept -> BufferPointer
{
    // TODO: Implement this function
    return nullptr;
}
