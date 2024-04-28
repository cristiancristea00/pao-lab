#include "AES.hpp"

#include <algorithm>
#include <format>
#include <fstream>
#include <random>
#include <string>


#define INT128_PTRC(x)    reinterpret_cast<int128_t const *>(x)
#define INT128_PTR(x)     reinterpret_cast<int128_t *>(x)


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

auto AES::Encrypt(BufferType const * const plaintext) const noexcept -> BufferPointer
{
    auto result = std::make_unique<BufferType[]>(keySize);

    int128_t temp;

    temp = _mm_loadu_si128(INT128_PTRC(plaintext));
    temp = _mm_xor_si128(temp, INT128_PTRC(roundKeys.get())[0]);
    for (std::size_t round = 1; round < rounds; ++round)
    {
        temp = _mm_aesenc_si128(temp, INT128_PTRC(roundKeys.get())[round]);
    }
    temp = _mm_aesenclast_si128(temp, INT128_PTRC(roundKeys.get())[rounds]);
    _mm_storeu_si128(INT128_PTR(result.get()), temp);

    return result;
}

auto AES::Decrypt(BufferType const * const ciphertext) const noexcept -> BufferPointer
{
    auto result = std::make_unique<BufferType[]>(keySize);

    int128_t temp;

    temp = _mm_loadu_si128(INT128_PTRC(ciphertext));
    temp = _mm_xor_si128(temp, INT128_PTRC(roundKeys.get())[rounds]);
    for (std::size_t round = 1; round < rounds; ++round)
    {
        temp = _mm_aesdec_si128(temp, INT128_PTRC(roundKeys.get())[rounds + round]);
    }
    temp = _mm_aesdeclast_si128(temp, INT128_PTRC(roundKeys.get())[0]);
    _mm_storeu_si128(INT128_PTR(result.get()), temp);

    return result;
}

auto AES::EncryptFile(std::string_view const inputFileName, std::string_view const outputFileName) const -> std::uint8_t
{
    return EncryptDecryptFile(inputFileName, outputFileName, true);
}

auto AES::DecryptFile(std::string_view const inputFileName, std::string_view const outputFileName, std::uint8_t const lastBytes) const -> void
{
    EncryptDecryptFile(inputFileName, outputFileName, false, lastBytes);
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

auto AES::EncryptDecryptFile(std::string_view const inputFileName, std::string_view const outputFileName, bool const encrypt, std::uint8_t const lastBytes) const -> std::uint8_t
{
    std::ifstream file{inputFileName.data(), std::ios::binary};

    if (!file.is_open())
    {
        throw std::runtime_error{std::format("Failed to open input file: {}", inputFileName)};
    }

    std::vector<BufferType> input(std::istreambuf_iterator<char>(file), {});

    file.close();

    auto const remainingBytes = static_cast<uint8_t>(input.size() % keySize);

    if (remainingBytes != 0)
    {
        for (std::size_t idx = 0; idx < keySize - remainingBytes; ++idx)
        {
            input.push_back(0U);
        }
    }

    input.shrink_to_fit();

    auto const processFunction = encrypt ? &AES::EncryptSequence : &AES::DecryptSequence;
    auto const processed = (this->*processFunction)(input);

    std::ofstream outputFile{outputFileName.data(), std::ios::binary};

    if (!outputFile.is_open())
    {
        throw std::runtime_error{std::format("Failed to open output file: {}", outputFileName)};
    }

    if (encrypt)
    {
        for (BufferType const & elem : processed)
        {
            outputFile.write(reinterpret_cast<char const *>(&elem), 1);
        }
    }
    else
    {
        for (std::size_t idx = 0; idx < processed.size() - (BLOCK_SIZE - lastBytes); ++idx)
        {
            outputFile.write(reinterpret_cast<char const *>(&processed[idx]), 1);
        }
    }

    outputFile.close();

    return remainingBytes;
}

auto AES::EncryptSequence(std::vector<BufferType> const & input) const noexcept -> std::vector<BufferType>
{
    return EncryptDecryptSequence(input);
}

auto AES::DecryptSequence(std::vector<BufferType> const & input) const noexcept -> std::vector<BufferType>
{
    return EncryptDecryptSequence(input);
}

auto AES::EncryptDecryptSequence(std::vector<BufferType> const & input) const -> std::vector<BufferType>
{
    static const auto NONCE = GetNonce();
    int128_t const nonce = _mm_loadu_si128(INT128_PTRC(NONCE.get()));

    std::vector<BufferType> result(input.size(), 0U);

    auto const currentVal = std::make_unique<BufferType[]>(BLOCK_SIZE);

    #pragma omp parallel for
    for (std::size_t idx = 0; idx < input.size(); idx += BLOCK_SIZE)
    {
        int128_t current = _mm_xor_si128(nonce, _mm_set_epi64x(0, idx));
        _mm_storeu_si128(INT128_PTR(currentVal.get()), current);
        auto const processed = Encrypt(currentVal.get());
        current = _mm_loadu_si128(INT128_PTRC(processed.get()));
        int128_t inputBlock = _mm_loadu_si128(INT128_PTRC(input.data() + idx));
        current = _mm_xor_si128(current, inputBlock);
        _mm_storeu_si128(INT128_PTR(result.data() + idx), current);
    }

    return result;
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
        key[idx / 2] = static_cast<BufferType>(std::stoul(std::string{stringKey.substr(idx, 2)}, nullptr, 16));
    }

    return key;
}

auto AES::KeyExpansion128(std::string_view const stringKey) noexcept -> BufferPointer
{
    auto const key = GetKeyFromString(stringKey);

    auto constexpr totalSize = (2 * as_num(RoundSize::AES128) - 2) * as_num(KeyType::AES128);
    auto roundKeys = std::make_unique<BufferType[]>(totalSize);

    int128_t temp1;
    int128_t temp2;

    auto * const keySchedule = INT128_PTR(roundKeys.get());

    temp1 = _mm_loadu_si128(INT128_PTRC(key.get()));
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

    /*
     *  Compute the Decryption Round Keys
     */

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
    auto const key = GetKeyFromString(stringKey);

    auto constexpr totalSize = (2 * as_num(RoundSize::AES192) - 2) * as_num(KeyType::AES192);
    auto roundKeys = std::make_unique<BufferType[]>(totalSize);

    int128_t temp1;
    int128_t temp2;
    int128_t temp3;

    auto * const keySchedule = INT128_PTR(roundKeys.get());

    temp1 = _mm_loadu_si128(INT128_PTRC(key.get()));
    temp3 = _mm_loadu_si128(INT128_PTRC(key.get() + 16));
    keySchedule[0] = temp1;
    keySchedule[1] = temp3;
    temp2 = _mm_aeskeygenassist_si128(temp3, 0x01);
    KeyExpansionAssist192(&temp1, &temp2, &temp3);
    keySchedule[1] = int128_t(_mm_shuffle_pd(double128_t(keySchedule[1]), double128_t(temp1), 0));
    keySchedule[2] = int128_t(_mm_shuffle_pd(double128_t(temp1), double128_t(temp3), 1));
    temp2 = _mm_aeskeygenassist_si128(temp3, 0x02);
    KeyExpansionAssist192(&temp1, &temp2, &temp3);
    keySchedule[3] = temp1;
    keySchedule[4] = temp3;
    temp2 = _mm_aeskeygenassist_si128(temp3, 0x04);
    KeyExpansionAssist192(&temp1, &temp2, &temp3);
    keySchedule[4] = int128_t(_mm_shuffle_pd(double128_t(keySchedule[4]), double128_t(temp1), 0));
    keySchedule[5] = int128_t(_mm_shuffle_pd(double128_t(temp1), double128_t(temp3), 1));
    temp2 = _mm_aeskeygenassist_si128(temp3, 0x08);
    KeyExpansionAssist192(&temp1, &temp2, &temp3);
    keySchedule[6] = temp1;
    keySchedule[7] = temp3;
    temp2 = _mm_aeskeygenassist_si128(temp3, 0x10);
    KeyExpansionAssist192(&temp1, &temp2, &temp3);
    keySchedule[7] = int128_t(_mm_shuffle_pd(double128_t(keySchedule[7]), double128_t(temp1), 0));
    keySchedule[8] = int128_t(_mm_shuffle_pd(double128_t(temp1), double128_t(temp3), 1));
    temp2 = _mm_aeskeygenassist_si128(temp3, 0x20);
    KeyExpansionAssist192(&temp1, &temp2, &temp3);
    keySchedule[9] = temp1;
    keySchedule[10] = temp3;
    temp2 = _mm_aeskeygenassist_si128(temp3, 0x40);
    KeyExpansionAssist192(&temp1, &temp2, &temp3);
    keySchedule[10] = int128_t(_mm_shuffle_pd(double128_t(keySchedule[10]), double128_t(temp1), 0));
    keySchedule[11] = int128_t(_mm_shuffle_pd(double128_t(temp1), double128_t(temp3), 1));
    temp2 = _mm_aeskeygenassist_si128(temp3, 0x80);
    KeyExpansionAssist192(&temp1, &temp2, &temp3);
    keySchedule[12] = temp1;

    /*
     *  Compute the Decryption Round Keys
     */

    keySchedule[13] = _mm_aesimc_si128(keySchedule[11]);
    keySchedule[14] = _mm_aesimc_si128(keySchedule[10]);
    keySchedule[15] = _mm_aesimc_si128(keySchedule[9]);
    keySchedule[16] = _mm_aesimc_si128(keySchedule[8]);
    keySchedule[17] = _mm_aesimc_si128(keySchedule[7]);
    keySchedule[18] = _mm_aesimc_si128(keySchedule[6]);
    keySchedule[19] = _mm_aesimc_si128(keySchedule[5]);
    keySchedule[20] = _mm_aesimc_si128(keySchedule[4]);
    keySchedule[21] = _mm_aesimc_si128(keySchedule[3]);
    keySchedule[22] = _mm_aesimc_si128(keySchedule[2]);
    keySchedule[23] = _mm_aesimc_si128(keySchedule[1]);

    return roundKeys;
}

auto AES::KeyExpansionAssist192(int128_t * const temp1, int128_t * const temp2, int128_t * const temp3) noexcept -> void
{
    int128_t temp4;
    *temp2 = _mm_shuffle_epi32(*temp2, 0x55);
    temp4 = _mm_slli_si128(*temp1, 0x04);
    *temp1 = _mm_xor_si128(*temp1, temp4);
    temp4 = _mm_slli_si128(temp4, 0x04);
    *temp1 = _mm_xor_si128(*temp1, temp4);
    temp4 = _mm_slli_si128(temp4, 0x04);
    *temp1 = _mm_xor_si128(*temp1, temp4);
    *temp1 = _mm_xor_si128(*temp1, *temp2);
    *temp2 = _mm_shuffle_epi32(*temp1, 0xFF);
    temp4 = _mm_slli_si128(*temp3, 0x04);
    *temp3 = _mm_xor_si128(*temp3, temp4);
    *temp3 = _mm_xor_si128(*temp3, *temp2);
}

auto AES::KeyExpansion256(std::string_view const stringKey) noexcept -> BufferPointer
{
    auto const key = GetKeyFromString(stringKey);

    auto constexpr totalSize = (2 * as_num(RoundSize::AES256) - 2) * as_num(KeyType::AES256);
    auto roundKeys = std::make_unique<BufferType[]>(totalSize);

    int128_t temp1;
    int128_t temp2;
    int128_t temp3;

    auto * const keySchedule = INT128_PTR(roundKeys.get());

    temp1 = _mm_loadu_si128(INT128_PTRC(key.get()));
    temp3 = _mm_loadu_si128(INT128_PTRC(key.get() + 16));
    keySchedule[0] = temp1;
    keySchedule[1] = temp3;
    temp2 = _mm_aeskeygenassist_si128(temp3, 0x01);
    KeyExpansionFirstAssist256(&temp1, &temp2);
    keySchedule[2] = temp1;
    KeyExpansionSecondAssist256(&temp1, &temp3);
    keySchedule[3] = temp3;
    temp2 = _mm_aeskeygenassist_si128(temp3, 0x02);
    KeyExpansionFirstAssist256(&temp1, &temp2);
    keySchedule[4] = temp1;
    KeyExpansionSecondAssist256(&temp1, &temp3);
    keySchedule[5] = temp3;
    temp2 = _mm_aeskeygenassist_si128(temp3, 0x04);
    KeyExpansionFirstAssist256(&temp1, &temp2);
    keySchedule[6] = temp1;
    KeyExpansionSecondAssist256(&temp1, &temp3);
    keySchedule[7] = temp3;
    temp2 = _mm_aeskeygenassist_si128(temp3, 0x08);
    KeyExpansionFirstAssist256(&temp1, &temp2);
    keySchedule[8] = temp1;
    KeyExpansionSecondAssist256(&temp1, &temp3);
    keySchedule[9] = temp3;
    temp2 = _mm_aeskeygenassist_si128(temp3, 0x10);
    KeyExpansionFirstAssist256(&temp1, &temp2);
    keySchedule[10] = temp1;
    KeyExpansionSecondAssist256(&temp1, &temp3);
    keySchedule[11] = temp3;
    temp2 = _mm_aeskeygenassist_si128(temp3, 0x20);
    KeyExpansionFirstAssist256(&temp1, &temp2);
    keySchedule[12] = temp1;
    KeyExpansionSecondAssist256(&temp1, &temp3);
    keySchedule[13] = temp3;
    temp2 = _mm_aeskeygenassist_si128(temp3, 0x40);
    KeyExpansionFirstAssist256(&temp1, &temp2);
    keySchedule[14] = temp1;

    /*
     *  Compute the Decryption Round Keys
     */

    keySchedule[15] = _mm_aesimc_si128(keySchedule[13]);
    keySchedule[16] = _mm_aesimc_si128(keySchedule[12]);
    keySchedule[17] = _mm_aesimc_si128(keySchedule[11]);
    keySchedule[18] = _mm_aesimc_si128(keySchedule[10]);
    keySchedule[19] = _mm_aesimc_si128(keySchedule[9]);
    keySchedule[20] = _mm_aesimc_si128(keySchedule[8]);
    keySchedule[21] = _mm_aesimc_si128(keySchedule[7]);
    keySchedule[22] = _mm_aesimc_si128(keySchedule[6]);
    keySchedule[23] = _mm_aesimc_si128(keySchedule[5]);
    keySchedule[24] = _mm_aesimc_si128(keySchedule[4]);
    keySchedule[25] = _mm_aesimc_si128(keySchedule[3]);
    keySchedule[26] = _mm_aesimc_si128(keySchedule[2]);
    keySchedule[27] = _mm_aesimc_si128(keySchedule[1]);

    return roundKeys;
}

auto AES::KeyExpansionFirstAssist256(int128_t * const temp1, int128_t * const temp2) noexcept -> void
{
    int128_t temp4;
    *temp2 = _mm_shuffle_epi32(*temp2, 0xFF);
    temp4 = _mm_slli_si128(*temp1, 0x04);
    *temp1 = _mm_xor_si128(*temp1, temp4);
    temp4 = _mm_slli_si128(temp4, 0x04);
    *temp1 = _mm_xor_si128(*temp1, temp4);
    temp4 = _mm_slli_si128(temp4, 0x04);
    *temp1 = _mm_xor_si128(*temp1, temp4);
    *temp1 = _mm_xor_si128(*temp1, *temp2);
}

auto AES::KeyExpansionSecondAssist256(int128_t * const temp1, int128_t * const temp3) noexcept -> void
{
    int128_t temp2;
    int128_t temp4;
    temp4 = _mm_aeskeygenassist_si128(*temp1, 0x00);
    temp2 = _mm_shuffle_epi32(temp4, 0xAA);
    temp4 = _mm_slli_si128(*temp3, 0x04);
    *temp3 = _mm_xor_si128(*temp3, temp4);
    temp4 = _mm_slli_si128(temp4, 0x04);
    *temp3 = _mm_xor_si128(*temp3, temp4);
    temp4 = _mm_slli_si128(temp4, 0x04);
    *temp3 = _mm_xor_si128(*temp3, temp4);
    *temp3 = _mm_xor_si128(*temp3, temp2);
}

auto AES::GetNonce() -> BufferPointer
{
    std::mt19937_64 randomEngine{SEED};
    std::uniform_int_distribution<BufferType> randomDistribution{0, std::numeric_limits<BufferType>::max()};

    auto result = std::make_unique<BufferType[]>(BLOCK_SIZE);

    for (std::size_t idx = 0; idx < BLOCK_SIZE; ++idx)
    {
        result[idx] = randomDistribution(randomEngine);
    }

    return result;
}
