#include <iostream>
#include <functional>
#include <chrono>

#include "TDES.hpp"


auto MeasureTime(std::function<void(void)> const & function, std::string_view const message) -> void;


auto main() -> int
{
    TDES tdes(TDES::GetRandomKey());

    MeasureTime([&] {
        tdes.EncryptFile("../plain.txt", "../encrypted.txt");
    }, "Encryption");

    MeasureTime([&] {
        tdes.DecryptFile("../encrypted.txt", "../decrypted.txt");
    }, "Decryption");

    return EXIT_SUCCESS;
}

auto MeasureTime(std::function<void(void)> const & function, std::string_view const message) -> void
{
    auto const start = std::chrono::high_resolution_clock::now();
    function();
    auto const stop = std::chrono::high_resolution_clock::now();
    auto const difference_ms = duration_cast<std::chrono::milliseconds>(stop - start);
    auto const time_ms = difference_ms.count();
    std::cout << std::format("{}: {} ms\n", message, time_ms);
}
