/*
## Processor

Name: Intel® Core™ i5-6600K
Cores: 4
Threads: 4
Base Frequency: 3.5 GHz
Max Frequency: 3.9 GHz
Cache: 6 MB
Memory Channels: 2
Max Memory Bandwidth: 34.1 GB/s

## Memory

Name: Corsair Vengeance LPX
Type: DDR4
Size: 16 GB (Dual Channel - 2x8 GB)
Speed: 3200 MT/s
Latency (Timings): 16-18-18-36

## Environment

Operating System: Ubuntu 23.10 (Mantic Minotaur)
Kernel: 6.5.0-21-generic
Compiler: gcc 13.2.0
*/

/*
File Size: 5.5 MB
Execution Time (Compiler Optimized): TBD ms
*/

#include <chrono>
#include <functional>
#include <iostream>

#include "AES.hpp"

auto MeasureTime(std::function<void()> const & function, std::string_view const message) -> void;


auto main() -> int
{
    // AES const aes(AES::GetRandomKey());
    //
    // MeasureTime(
    //     [&]
    //     {
    //         aes.EncryptFile("../plain.txt", "../encrypted.txt");
    //     }, "Encryption"
    // );
    //
    // MeasureTime(
    //     [&]
    //     {
    //         aes.DecryptFile("../encrypted.txt", "../decrypted.txt");
    //     }, "Decryption"
    // );

    AES const aes("2b7e151628aed2a6abf7158809cf4f3c");

    // const auto * plaintext = reinterpret_cast<BufferType const *>("\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a");
    const auto * plaintext = reinterpret_cast<BufferType const *>("\x3a\xd7\x7b\xb4\x0d\x7a\x36\x60\xa8\x9e\xca\xf3\x24\x66\xef\x97");

    auto const result = aes.Decrypt(const_cast<BufferType *>(plaintext));

    std::string resultString;

    for (std::size_t idx = 0; idx < 16; ++idx)
    {
        resultString.append(std::format("{:02X}", result[idx]));
    }

    std::cout << resultString << '\n';

    return EXIT_SUCCESS;
}

auto MeasureTime(std::function<void()> const & function, std::string_view const message) -> void
{
    auto const start = std::chrono::high_resolution_clock::now();
    function();
    auto const stop = std::chrono::high_resolution_clock::now();
    auto const difference_ms = duration_cast<std::chrono::milliseconds>(stop - start);
    auto const time_ms = difference_ms.count();
    std::cout << std::format("{}: {} ms\n", message, time_ms);
}
