# Reverse

## Processor

*Name*: **Intel® Core™ i5-6600K**

*Cores*: **4**

*Threads*: **4**

*Base Frequency*: **3.50 GHz**

*Max Frequency*: **3.90 GHz**

*Caches*: **6 MB**

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

***

## Results

### Naive

[`Jump to source code`](ReverseNaive/main.cpp#L75)

*Execution Time*: **5634 ms**

*Execution Time (Compiler Optimized)*: **3030 ms**

### Single LUT

#### 32-bit LUT

[`Jump to source code`](ReverseSingleLUT/main.cpp#L185)

*Execution Time*: **Out of Memory**

*Execution Time (Compiler Optimized)*: **Out of Memory**

#### 16-bit LUT

[`Jump to source code`](ReverseSingleLUT/main.cpp#L193)

*Execution Time*: **479 ms**

*Execution Time (Compiler Optimized)*: **322 ms**

#### 8-bit LUT

[`Jump to source code`](ReverseSingleLUT/main.cpp#L212)

*Execution Time*: **798 ms**

*Execution Time (Compiler Optimized)*: **500 ms**

#### 4-bit LUT

[`Jump to source code`](ReverseSingleLUT/main.cpp#L237)

*Execution Time*: **1350 ms**

*Execution Time (Compiler Optimized)*: **821 ms**

### Multiple LUTs

#### 32-bit LUT

[`Jump to source code`](ReverseLUT/main.cpp#L234)

*Execution Time*: **Out of Memory**

*Execution Time (Compiler Optimized)*: **Out of Memory**

#### 16-bit LUT

[`Jump to source code`](ReverseLUT/main.cpp#L242)

*Execution Time*: **550 ms**

*Execution Time (Compiler Optimized)*: **428 ms**

#### 8-bit LUT

[`Jump to source code`](ReverseLUT/main.cpp#L250)

*Execution Time*: **548 ms**

*Execution Time (Compiler Optimized)*: **295 ms**

#### 4-bit LUT

[`Jump to source code`](ReverseLUT/main.cpp#L259)

*Execution Time*: **861 ms**

*Execution Time (Compiler Optimized)*: **422 ms**
