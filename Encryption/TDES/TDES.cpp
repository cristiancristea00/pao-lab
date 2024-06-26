#include "TDES.hpp"

#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <random>

#include "DES.hpp"


auto MeasureTime(std::function<void()> const & function, std::string_view const message) -> void
{
    auto const start = std::chrono::high_resolution_clock::now();
    function();
    auto const stop = std::chrono::high_resolution_clock::now();
    auto const difference_ms = duration_cast<std::chrono::milliseconds>(stop - start);
    auto const time_ms = difference_ms.count();
    std::cout << std::format("{}: {} ms\n", message, time_ms);
}


TDES::TDES(std::string_view const key) noexcept: des1(std::make_unique<DES>(key.substr(0, KEY_SIZE_IN_BYTES / NUM_KEYS * LENGTH_RATIO))),
                                                 des2(std::make_unique<DES>(key.substr(KEY_SIZE_IN_BYTES / NUM_KEYS * LENGTH_RATIO, KEY_SIZE_IN_BYTES / NUM_KEYS * LENGTH_RATIO))),
                                                 des3(std::make_unique<DES>(key.substr(KEY_SIZE_IN_BYTES / NUM_KEYS * LENGTH_RATIO * 2, KEY_SIZE_IN_BYTES / NUM_KEYS * LENGTH_RATIO))) { }

auto TDES::Encrypt(std::uint64_t const plaintext) const noexcept -> std::uint64_t
{
    auto const firstRound = des1->Encrypt(plaintext);
    auto const secondRound = des2->Decrypt(firstRound);
    auto const thirdRound = des3->Encrypt(secondRound);

    return thirdRound;
}

auto TDES::Decrypt(std::uint64_t const ciphertext) const noexcept -> std::uint64_t
{
    auto const firstRound = des3->Decrypt(ciphertext);
    auto const secondRound = des2->Encrypt(firstRound);
    auto const thirdRound = des1->Decrypt(secondRound);

    return thirdRound;
}

auto TDES::EncryptFile(std::string_view const inputFileName, std::string_view const outputFileName) const -> std::uint8_t
{
    return EncryptDecryptFile(inputFileName, outputFileName, true);
}

auto TDES::DecryptFile(std::string_view const inputFileName, std::string_view const outputFileName, std::uint8_t const lastBytes) const -> void
{
    EncryptDecryptFile(inputFileName, outputFileName, false, lastBytes);
}

auto TDES::GetRandomKey() noexcept -> std::string
{
    std::mt19937_64 randomEngine{SEED};
    std::uniform_int_distribution<std::uint8_t> randomDistribution{0, std::numeric_limits<std::uint8_t>::max()};

    std::string key;
    key.reserve(KEY_SIZE_IN_BYTES);

    for (std::size_t idx = 0; idx < KEY_SIZE_IN_BYTES; ++idx)
    {
        key.append(std::format("{:02X}", randomDistribution(randomEngine)));
    }

    return key;
}

auto TDES::EncryptDecryptFile(std::string_view const inputFileName, std::string_view const outputFileName, bool const encrypt, std::uint8_t const lastBytes) const -> std::uint8_t
{
    std::ifstream file{inputFileName.data(), std::ios::binary};

    if (!file.is_open())
    {
        throw std::runtime_error{std::format("Failed to open input file: {}", inputFileName)};
    }

    std::vector<std::uint8_t> buffer(std::istreambuf_iterator<char>(file), {});

    file.close();

    std::uint8_t const remainingBytes = buffer.size() % BYTES_IN_64BITS;

    if (remainingBytes != 0)
    {
        for (std::size_t idx = 0; idx < BYTES_IN_64BITS - remainingBytes; ++idx)
        {
            buffer.push_back(0U);
        }
    }

    std::vector<std::uint64_t> input;
    input.reserve(buffer.size() / BYTES_IN_64BITS);

    std::uint64_t currentValue{0U};

    for (auto idx = 0U; idx < buffer.size(); idx += BYTES_IN_64BITS)
    {
        if constexpr (std::endian::native == std::endian::little)
        {
            currentValue = static_cast<std::uint64_t>(buffer[idx + 7]) << 56U | static_cast<std::uint64_t>(buffer[idx + 6]) << 48U |
                static_cast<std::uint64_t>(buffer[idx + 5]) << 40U | static_cast<std::uint64_t>(buffer[idx + 4]) << 32U |
                static_cast<std::uint64_t>(buffer[idx + 3]) << 24U | static_cast<std::uint64_t>(buffer[idx + 2]) << 16U |
                static_cast<std::uint64_t>(buffer[idx + 1]) << 8U | static_cast<std::uint64_t>(buffer[idx + 0]);
        }
        else
        {
            currentValue = static_cast<std::uint64_t>(buffer[idx + 0]) << 56U | static_cast<std::uint64_t>(buffer[idx + 1]) << 48U |
                static_cast<std::uint64_t>(buffer[idx + 2]) << 40U | static_cast<std::uint64_t>(buffer[idx + 3]) << 32U |
                static_cast<std::uint64_t>(buffer[idx + 4]) << 24U | static_cast<std::uint64_t>(buffer[idx + 5]) << 16U |
                static_cast<std::uint64_t>(buffer[idx + 6]) << 8U | static_cast<std::uint64_t>(buffer[idx + 7]);
        }

        input.push_back(currentValue);
    }

    buffer.clear();
    buffer.shrink_to_fit();

    auto const processFunction = encrypt ? &TDES::EncryptSequence : &TDES::DecryptSequence;
    std::vector<std::uint64_t> processed;

    MeasureTime(
        [&] -> void
        {
            processed = (this->*processFunction)(input);
        }, encrypt ? "Encryption" : "Decryption"
    );

    std::ofstream outputFile{outputFileName.data(), std::ios::binary};

    if (!outputFile.is_open())
    {
        throw std::runtime_error{std::format("Failed to open output file: {}", outputFileName)};
    }

    if (encrypt)
    {
        outputFile.write(reinterpret_cast<char const *>(processed.data()), processed.size() * BYTES_IN_64BITS);
    }
    else
    {
        outputFile.write(reinterpret_cast<char const *>(processed.data()), (processed.size() - 1) * BYTES_IN_64BITS);
        outputFile.write(reinterpret_cast<char const *>(&processed.back()), lastBytes == 0 ? BYTES_IN_64BITS : lastBytes);
    }

    outputFile.close();

    return remainingBytes;
}

auto TDES::EncryptSequence(std::vector<std::uint64_t> const & input) const noexcept -> std::vector<std::uint64_t>
{
    return EncryptDecryptSequence(input);
}

auto TDES::DecryptSequence(std::vector<std::uint64_t> const & input) const noexcept -> std::vector<std::uint64_t>
{
    return EncryptDecryptSequence(input);
}

auto TDES::EncryptDecryptSequence(std::vector<std::uint64_t> const & input) const -> std::vector<std::uint64_t>
{
    static const auto NONCE = GetNonce();

    std::vector<std::uint64_t> result(input.size(), 0U);

    #pragma omp parallel for default(none) shared(input, result, NONCE) schedule(static)
    for (std::size_t idx = 0; idx < input.size(); ++idx)
    {
        result[idx] = input[idx] ^ Encrypt(NONCE ^ idx);
    }

    return result;
}

auto TDES::GetNonce() -> std::uint64_t
{
    std::mt19937_64 randomEngine{SEED};
    std::uniform_int_distribution<std::uint64_t> randomDistribution{0, std::numeric_limits<std::uint64_t>::max()};

    return randomDistribution(randomEngine);
}
