#include <iostream>

#include "TDES.hpp"


auto main() -> int
{
    TDES tdes(TDES::GetRandomKey());
    tdes.EncryptFile("../plain.txt", "../encrypted.txt");
    tdes.DecryptFile("../encrypted.txt", "../decrypted.txt");

    return EXIT_SUCCESS;
}
