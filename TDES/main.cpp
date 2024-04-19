#include <iostream>

#include "TDES.hpp"


auto main() -> int
{
    auto const key = TDES::GetRandomKey();
    TDES::EncryptFile("../plain.txt", "../encrypted.txt", key);
    TDES::DecryptFile("../encrypted.txt", "../decrypted.txt", key);

    return EXIT_SUCCESS;
}
