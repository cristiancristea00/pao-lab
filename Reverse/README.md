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

***

## Results

### Naive

*Execution Time*: **5634 ms**

*Execution Time (Compiler Optimized)*: **3030 ms**

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

### OpenMP - No LUT

#### Without Manual Loop Unrolling

*Execution Time (Compiler Optimized)*: **94 ms**

#### With Manual Loop Unrolling

*Execution Time (Compiler Optimized)*: **95 ms**

> **Note**: The compiler already unrolls the loop, so manual unrolling does not improve performance.
