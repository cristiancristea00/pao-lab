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
Execution Time (Compiler Optimized): 290 ms
*/

#include <chrono>
#include <functional>
#include <iostream>

#include "TDES.hpp"


auto main() -> int
{
    TDES const tdes(TDES::GetRandomKey());

    auto const lastBytes = tdes.EncryptFile("../plain.txt", "../encrypted.txt");

    tdes.DecryptFile("../encrypted.txt", "../decrypted.txt", lastBytes);

    return EXIT_SUCCESS;
}
