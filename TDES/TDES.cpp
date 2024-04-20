#include "TDES.hpp"
#include "DES.hpp"

#include <fstream>
#include <format>
#include <random>

TDES::TDES(std::string_view const key) noexcept:
    des1(std::make_unique<DES>(key.substr(0, KEY_SIZE_IN_BYTES / NUM_KEYS * LENGTH_RATIO))),
    des2(std::make_unique<DES>(key.substr(KEY_SIZE_IN_BYTES / NUM_KEYS * LENGTH_RATIO, KEY_SIZE_IN_BYTES / NUM_KEYS * LENGTH_RATIO))),
    des3(std::make_unique<DES>(key.substr(KEY_SIZE_IN_BYTES / NUM_KEYS * LENGTH_RATIO * 2, KEY_SIZE_IN_BYTES / NUM_KEYS * LENGTH_RATIO))) { }

auto TDES::Encrypt(uint64_t const plaintext) const noexcept -> uint64_t
{
    auto const firstRound = des1->Encrypt(plaintext);
    auto const secondRound = des2->Decrypt(firstRound);
    auto const thirdRound = des3->Encrypt(secondRound);

    return thirdRound;
}

auto TDES::Decrypt(uint64_t const ciphertext) const noexcept -> uint64_t
{
    auto const firstRound = des3->Decrypt(ciphertext);
    auto const secondRound = des2->Encrypt(firstRound);
    auto const thirdRound = des1->Decrypt(secondRound);

    return thirdRound;
}

auto TDES::EncryptFile(std::string_view const inputFileName, std::string_view const outputFileName) const -> void
{
    EncryptDecryptFile(inputFileName, outputFileName, true);
}

auto TDES::DecryptFile(std::string_view const inputFileName, std::string_view const outputFileName) const -> void
{
    EncryptDecryptFile(inputFileName, outputFileName, false);
}

auto TDES::GetRandomKey() noexcept -> std::string
{
    std::mt19937_64 randomEngine{SEED};
    std::uniform_int_distribution<uint8_t> randomDistribution{0, std::numeric_limits<uint8_t>::max()};

    std::string key;
    key.reserve(KEY_SIZE_IN_BYTES);

    for (std::size_t idx = 0; idx < KEY_SIZE_IN_BYTES; ++idx)
    {
        key.append(std::format("{:02X}", randomDistribution(randomEngine)));
    }

    return key;
}

auto TDES::EncryptDecryptFile(std::string_view const inputFileName, std::string_view const outputFileName, bool const encrypt) const -> void
{
    std::ifstream file{inputFileName.data(), std::ios::binary};

    if (!file.is_open())
    {
        throw std::runtime_error{std::format("Failed to open input file: {}", inputFileName)};
    }

    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});

    file.close();

    std::size_t const remainingBytes = buffer.size() % BYTES_IN_64BITS;

    for (std::size_t idx = 0; idx < remainingBytes; ++idx)
    {
        buffer.push_back(0U);
    }

    std::vector<uint64_t> input;
    input.reserve(buffer.size() / BYTES_IN_64BITS);

    uint64_t currentValue{0U};

    for (auto idx = 0U; idx < buffer.size(); idx += BYTES_IN_64BITS)
    {
        if constexpr (std::endian::native == std::endian::little)
        {
            currentValue = static_cast<uint64_t>(buffer[idx + 7]) << 56U | static_cast<uint64_t>(buffer[idx + 6]) << 48U |
                           static_cast<uint64_t>(buffer[idx + 5]) << 40U | static_cast<uint64_t>(buffer[idx + 4]) << 32U |
                           static_cast<uint64_t>(buffer[idx + 3]) << 24U | static_cast<uint64_t>(buffer[idx + 2]) << 16U |
                           static_cast<uint64_t>(buffer[idx + 1]) << 8U | static_cast<uint64_t>(buffer[idx + 0]);
        }
        else
        {
            currentValue = static_cast<uint64_t>(buffer[idx + 0]) << 56U | static_cast<uint64_t>(buffer[idx + 1]) << 48U |
                           static_cast<uint64_t>(buffer[idx + 2]) << 40U | static_cast<uint64_t>(buffer[idx + 3]) << 32U |
                           static_cast<uint64_t>(buffer[idx + 4]) << 24U | static_cast<uint64_t>(buffer[idx + 5]) << 16U |
                           static_cast<uint64_t>(buffer[idx + 6]) << 8U | static_cast<uint64_t>(buffer[idx + 7]);
        }

        input.push_back(currentValue);
    }

    buffer.clear();
    buffer.shrink_to_fit();

    auto const processFunction = encrypt ? &TDES::EncryptSequence : &TDES::DecryptSequence;
    auto const processed = (this->*processFunction)(input);

    std::ofstream outputFile{outputFileName.data(), std::ios::binary};

    if (!outputFile.is_open())
    {
        throw std::runtime_error{std::format("Failed to open output file: {}", outputFileName)};
    }

    for (uint64_t const elem: processed)
    {
        outputFile.write(reinterpret_cast<char const *>(&elem), BYTES_IN_64BITS);
    }

    outputFile.close();
}

auto TDES::EncryptSequence(std::vector<uint64_t> const & input) const noexcept -> std::vector<uint64_t>
{
    return EncryptDecryptSequence(input);
}

auto TDES::DecryptSequence(std::vector<uint64_t> const & input) const noexcept -> std::vector<uint64_t>
{
    return EncryptDecryptSequence(input);
}

auto TDES::EncryptDecryptSequence(std::vector<uint64_t> const & input) const -> std::vector<uint64_t>
{
    static const std::size_t NONCE = GetNonce();

    std::vector<uint64_t> result(input.size(), 0U);

    #pragma omp parallel for
    for (std::size_t idx = 0; idx < input.size(); ++idx)
    {
        result[idx] = input[idx] ^ Encrypt(NONCE ^ idx);
    }

    return result;
}

auto TDES::GetNonce() -> uint64_t
{
    std::mt19937_64 randomEngine{SEED};
    std::uniform_int_distribution<uint64_t> randomDistribution{0, std::numeric_limits<uint64_t>::max()};

    return randomDistribution(randomEngine);
}