# Reverse

## Processor

*Name*: **Intel® Core™ i5-6600K**

*Cores*: **4**

*Threads*: **4**

*Base Frequency*: **3.5 GHz**

*Max Frequency*: **3.9 GHz**

*Cache*: **6 MB**

*Memory Channels*: **2**

*Max Memory Bandwidth*: **34.1 GB/s**

## Memory

*Name*: **Corsair Vengeance LPX**

*Type*: **DDR4**

*Size*: **16 GB (Dual Channel - 2x8 GB)**

*Speed*: **3200 MT/s**

*Latency (Timings)*: **16-18-18-36**

## Environment

*Operating System*: **Ubuntu 23.10 (Mantic Minotaur)**

*Kernel*: **6.5.0-21-generic**

*Compiler*: **gcc 13.2.0**

## Results

### Naive

*Execution Time*: **4715 ms**

*Execution Time (Compiler Optimized)*: **365 ms**

***

### Single LUT

#### 32-bit LUT

*Space*: **16 GiB**

*Execution Time*: **Out of Memory**

*Execution Time (Compiler Optimized)*: **Out of Memory**

#### 16-bit LUT

*Space*: **128 KiB**

*Execution Time*: **594 ms**

*Execution Time (Compiler Optimized)*: **389 ms**

#### 8-bit LUT

*Space*: **256 B**

*Execution Time*: **798 ms**

*Execution Time (Compiler Optimized)*: **500 ms**

#### 4-bit LUT

*Space*: **8 B**

*Execution Time*: **1350 ms**

*Execution Time (Compiler Optimized)*: **821 ms**

***

### Multiple LUTs

#### 32-bit LUTs

*Space*: **16 GiB**

*Execution Time*: **Out of Memory**

*Execution Time (Compiler Optimized)*: **Out of Memory**

#### 16-bit LUTs

*Space*: **512 KiB**

*Execution Time*: **550 ms**

*Execution Time (Compiler Optimized)*: **428 ms**

#### 8-bit LUTs

*Space*: **4 KiB**

*Execution Time*: **548 ms**

*Execution Time (Compiler Optimized)*: **295 ms**

#### 4-bit LUTs

*Space*: **512 B**

*Execution Time*: **861 ms**

*Execution Time (Compiler Optimized)*: **422 ms**

***

### Threaded

#### Threaded Chunked

*Execution Time*: **1209 ms**

*Execution Time (Compiler Optimized)*: **96 ms**

#### Threaded Chunked - Unrolled

*Execution Time*: **345 ms**

*Execution Time (Compiler Optimized)*: **48 ms**

#### Threaded Interleaved

*Execution Time*: **1323 ms**

*Execution Time (Compiler Optimized)*: **378 ms**

#### Threaded Interleaved - Unrolled

*Execution Time*: **354 ms**

*Execution Time (Compiler Optimized)*: **212 ms**

***

### Threaded - Single LUT

#### 32-bit LUTs

*Space*: **16 GiB**

*Execution Time - Chunked*: **Out of Memory**

*Execution Time - Chunked (Compiler Optimized)*: **Out of Memory**

*Execution Time - Interleaved*: **Out of Memory**

*Execution Time - Interleaved (Compiler Optimized)*: **Out of Memory**

#### 16-bit LUTs

*Space*: **128 KiB**

*Execution Time - Chunked*: **159 ms**

*Execution Time - Chunked (Compiler Optimized)*: **106 ms**

*Execution Time - Interleaved*: **114 ms**

*Execution Time - Interleaved (Compiler Optimized)*: **60 ms**

#### 8-bit LUTs

*Space*: **256 B**

*Execution Time - Chunked*: **213 ms**

*Execution Time - Chunked (Compiler Optimized)*: **131 ms**

*Execution Time - Interleaved*: **165 ms**

*Execution Time - Interleaved (Compiler Optimized)*: **81 ms**

#### 4-bit LUTs

*Space*: **8 B**

*Execution Time - Chunked*: **352 ms**

*Execution Time - Chunked (Compiler Optimized)*: **210 ms**

*Execution Time - Interleaved*: **298 ms**

*Execution Time - Interleaved (Compiler Optimized)*: **165 ms**

***

### Threaded - Multiple LUTs

#### 32-bit LUTs

*Space*: **16 GiB**

*Execution Time - Chunked*: **Out of Memory**

*Execution Time - Chunked (Compiler Optimized)*: **Out of Memory**

*Execution Time - Interleaved*: **Out of Memory**

*Execution Time - Interleaved (Compiler Optimized)*: **Out of Memory**

#### 16-bit LUTs

*Space*: **512 KiB**

*Execution Time - Chunked*: **153 ms**

*Execution Time - Chunked (Compiler Optimized)*: **113 ms**

*Execution Time - Interleaved*: **117 ms**

*Execution Time - Interleaved (Compiler Optimized)*: **86 ms**

#### 8-bit LUTs

*Space*: **4 KiB**

*Execution Time - Chunked*: **149 ms**

*Execution Time - Chunked (Compiler Optimized)*: **89 ms**

*Execution Time - Interleaved*: **102 ms**

*Execution Time - Interleaved (Compiler Optimized)*: **42 ms**

#### 4-bit LUTs

*Space*: **512 B**

*Execution Time - Chunked*: **227 ms**

*Execution Time - Chunked (Compiler Optimized)*: **111 ms**

*Execution Time - Interleaved*: **179 ms**

*Execution Time - Interleaved (Compiler Optimized)*: **66 ms**

***

### OpenMP - No LUT

#### Without Manual Loop Unrolling

*Execution Time*: **15236 ms**

*Execution Time (Compiler Optimized)*: **94 ms**

#### With Manual Loop Unrolling

*Execution Time*: **2548 ms**

*Execution Time (Compiler Optimized)*: **95 ms**

***

### OpenMP - Single LUT

#### 32-bit LUT

*Space*: **16 GiB**

*Execution Time*: **Out of Memory**

*Execution Time (Compiler Optimized)*: **Out of Memory**

#### 16-bit LUT

*Space*: **128 KiB**

*Execution Time*: **2102 ms**

*Execution Time (Compiler Optimized)*: **468 ms**

#### 8-bit LUT

*Space*: **256 B**

*Execution Time*: **2817 ms**

*Execution Time (Compiler Optimized)*: **669 ms**

#### 4-bit LUT

*Space*: **8 B**

*Execution Time*: **5225 ms**

*Execution Time (Compiler Optimized)*: **1186 ms**

***

### OpenMP - Multiple LUTs

#### 32-bit LUTs

*Space*: **16 GiB**

*Execution Time*: **Out of Memory**

*Execution Time (Compiler Optimized)*: **Out of Memory**

#### 16-bit LUTs

*Space*: **512 KiB**

*Execution Time*: **160 ms**

*Execution Time (Compiler Optimized)*: **109 ms**

#### 8-bit LUTs

*Space*: **4 KiB**

*Execution Time*: **156 ms**

*Execution Time (Compiler Optimized)*: **79 ms**

#### 4-bit LUTs

*Space*: **512 B**

*Execution Time*: **251 ms**

*Execution Time (Compiler Optimized)*: **110 ms**
