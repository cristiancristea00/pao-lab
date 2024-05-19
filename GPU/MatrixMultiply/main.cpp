/*
## Processor

Name: Intel® Core™ i7-9750H
Cores: 6
Threads: 12
Base Frequency: 2.6 GHz
Max Frequency: 4.5 GHz
Cache: 12 MB
Memory Channels: 2
Max Memory Bandwidth: 41.8 GB/s

## Graphics
Name: NVIDIA GeForce GTX 1650
Compute Capability: 7.5
CUDA Cores: 1024
Stream Multiprocessors: 16
L1 Cache: 64 KB (per SM)
L2 Cache: 1024 KB
Base Frequency: 1455 MHz
Boost Frequency: 1620 MHz
Memory Size: 4 GB
Memory Type: GDDR6
Memory Bus: 128-bit
Memory Bandwidth: 192 GB/s
Memory Clock: 1500 MT/s (12 Gbps effective)
Performance: 3.318 TFLOPS (FP32)

## Memory

Type: DDR4
Size: 16 GB (Dual Channel - 2x8 GB)
Speed: 2667 MT/s

## Environment

Operating System: Ubuntu 24.04 (Noble Numbat)
Kernel: 6.8.0-31-generic
Compiler: gcc 14.0.1
*/

/*
Execution Time (Compiler Optimized): 1293 ms => 0.8304 TFLOPS
*/


#include <print>
#include <ranges>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <numeric>

#include <CL/opencl.hpp>


#define SEED         ( 0xDEADBEEF42UL )

#define M            ( 10'240UL )
#define N            ( 10'240UL )
#define K            ( 10'240UL )
#define SIZE_A       ( M * K )
#define SIZE_B       ( K * N )
#define SIZE_C       ( M * N )
#define STORAGE_A    ( SIZE_A * sizeof(float) )
#define STORAGE_B    ( SIZE_B * sizeof(float) )
#define STORAGE_C    ( SIZE_C * sizeof(float) )

#define TILE_SIZE_M           ( 128 )
#define TILE_SIZE_N           ( 128 )
#define TILE_SIZE_K           ( 32 )
#define WORK_THREAD_M         ( 8 )
#define WORK_THREAD_N         ( 8 )
#define RED_TILE_SIZE_M       ( TILE_SIZE_M / WORK_THREAD_M )
#define RED_TILE_SIZE_N       ( TILE_SIZE_N / WORK_THREAD_N )
#define LOADS_PER_THREAD_A    ( ( TILE_SIZE_K * TILE_SIZE_M ) / ( RED_TILE_SIZE_M * RED_TILE_SIZE_N ) )
#define LOADS_PER_THREAD_B    ( ( TILE_SIZE_K * TILE_SIZE_N ) / ( RED_TILE_SIZE_M * RED_TILE_SIZE_N ) )


auto GetContext() -> cl::Context;
auto GetProgram(cl::Context const & context, std::string_view const filename) -> cl::Program;

auto MeasureTime(std::function<void()> const & function, std::string_view const message) -> void;


using namespace std::chrono;


auto main() -> int
{
    cl::Context const context = GetContext();
    cl::Program const program = GetProgram(context, "../multiply.cl");

    std::mt19937 randomEngine{SEED};
    std::uniform_real_distribution<float> randomDistribution{0.0, 1.0};
    auto generator = [&]() -> float { return randomDistribution(randomEngine); };

    std::vector<float> mat1(SIZE_A);
    std::vector<float> mat2(SIZE_B);
    std::vector<float> result(SIZE_C);

    std::generate(mat1.begin(), mat1.end(), generator);
    std::generate(mat2.begin(), mat2.end(), generator);

    cl::Buffer bufferA{context, CL_MEM_READ_WRITE, STORAGE_A};
    cl::Buffer bufferB{context, CL_MEM_READ_WRITE, STORAGE_B};
    cl::Buffer bufferC{context, CL_MEM_WRITE_ONLY, STORAGE_C};

    cl::CommandQueue queue{context};

    MeasureTime([&] {
        cl::copy(queue, mat1.begin(), mat1.end(), bufferA);
        cl::copy(queue, mat2.begin(), mat2.end(), bufferB);
    }, "Time taken to load data");

    cl::KernelFunctor<cl::Buffer, cl::Buffer, cl::Buffer, unsigned const, unsigned const, unsigned const> multiplyFunctor{program, "multiply"};
    cl::EnqueueArgs const args{queue, cl::NDRange{M / WORK_THREAD_M, N / WORK_THREAD_N}, cl::NDRange{RED_TILE_SIZE_M, RED_TILE_SIZE_N}};

    MeasureTime([&] {
        multiplyFunctor(args, bufferA, bufferB, bufferC, M, N, K);
        queue.finish();
    }, "Time taken for matrix multiplication");

    MeasureTime([&] {
        cl::copy(queue, bufferC, result.begin(), result.end());
    }, "Time taken to unload data");

    auto const checksum = std::reduce(result.cbegin(), result.cend(), 0.0F, [] (auto const & lhs, auto const & rhs) -> auto { return (lhs + rhs) / 100.0F; });

    std::println("Checksum: {}", checksum);

    return 0;
}


auto GetContext() -> cl::Context
{
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty())
    {
        std::println(stderr, "No OpenCL platforms found. Check your OpenCL and drivers installation.");
        return EXIT_FAILURE;
    }

    std::println("Found {} OpenCL platform(s):", platforms.size());
    for (auto const & [index, platform]: std::views::enumerate(platforms))
    {
        std::println("Platform {}: {}", index, platform.getInfo<CL_PLATFORM_NAME>());
    }

    cl::Platform const platform = platforms.front();

    std::println("\nUsing platform: {}", platform.getInfo<CL_PLATFORM_NAME>());

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

    if (devices.empty())
    {
        std::println(stderr, "No OpenCL devices found. Check your OpenCL and drivers installation.");
        return EXIT_FAILURE;
    }

    std::println("\nFound {} OpenCL device(s):", devices.size());
    for (auto const & [index, device]: std::views::enumerate(devices))
    {
        std::println("Device {}: {}", index, device.getInfo<CL_DEVICE_NAME>());
    }

    cl::Device const device = devices.front();
    std::println("\nUsing device: {}", device.getInfo<CL_DEVICE_NAME>());

    return cl::Context{device};
}

auto GetProgram(cl::Context const & context, std::string_view const filename) -> cl::Program
{
    std::ifstream file{filename.data()};

    using FileIterator = std::istreambuf_iterator<char>;
    std::string source{FileIterator{file}, FileIterator{}};

    cl::Program program{context, source};

    try
    {
        program.build();
    }
    catch(cl::Error const & error)
    {
        if (error.err() == CL_BUILD_PROGRAM_FAILURE)
        {
            std::println(stderr, "Build log:");
            std::println(stderr, "{}", program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(context.getInfo<CL_CONTEXT_DEVICES>().front()));
        }
        
        throw;
    }

    return program;
}


auto MeasureTime(std::function<void()> const & function, std::string_view const message) -> void
{
    auto const start = high_resolution_clock::now();
    function();
    auto const stop = high_resolution_clock::now();
    auto const difference_ms = duration_cast<milliseconds>(stop - start);
    auto const time_ms = difference_ms.count();
    std::println("{}: {} ms", message, time_ms);
}
