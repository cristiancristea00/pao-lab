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
Execution Time (Compiler Optimized): 56 ms => 0.686 TFLOPS
*/


#include <print>
#include <ranges>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <fstream>

#include <CL/opencl.hpp>


#define SEED    ( 0xDEADBEEF42UL )

#define DIMENSIONS         ( 128UL )
#define NUM_OF_POINTS      ( 10'000UL )
#define TOTAL_SIZE         ( NUM_OF_POINTS * DIMENSIONS )
#define STORAGE            ( TOTAL_SIZE * sizeof(float) )
#define STORAGE_INDICES    ( NUM_OF_POINTS * sizeof(std::size_t) )


auto GetContext() -> cl::Context;

auto GetProgram(cl::Context const & context, std::string_view const filename) -> cl::Program;

auto MeasureTime(std::function<void()> const & function, std::string_view const message) -> void;

auto ComputeChecksum(std::array<std::size_t, NUM_OF_POINTS> const & indices) -> void;


std::array<float, TOTAL_SIZE> set1{};
std::array<float, TOTAL_SIZE> set2{};
std::array<std::size_t, NUM_OF_POINTS> indices{};


auto main() -> int
{
    auto const context = GetContext();
    auto const program = GetProgram(context, "../ssd.cl");

    std::mt19937 randomEngine{SEED};
    std::uniform_real_distribution<float> randomDistribution{0.0, 1.0};
    auto const generator = [&]() -> float
    {
        return randomDistribution(randomEngine);
    };

    std::ranges::generate(set1, generator);
    std::ranges::generate(set2, generator);

    cl::Buffer bufferSet1{context, CL_MEM_READ_ONLY, STORAGE};
    cl::Buffer bufferSet2{context, CL_MEM_READ_ONLY, STORAGE};
    cl::Buffer bufferIndices{context, CL_MEM_WRITE_ONLY, STORAGE_INDICES};

    cl::CommandQueue queue{context};

    MeasureTime(
        [&] -> void
        {
            cl::copy(queue, set1.begin(), set1.end(), bufferSet1);
            cl::copy(queue, set2.begin(), set2.end(), bufferSet2);
        }, "Time taken to load data"
    );

    cl::KernelFunctor<cl::Buffer, cl::Buffer, cl::Buffer, unsigned const> ssdFunctor{program, "ssd"};
    cl::EnqueueArgs const args{queue, cl::NDRange{NUM_OF_POINTS}};

    MeasureTime(
        [&] -> void
        {
            ssdFunctor(args, bufferSet1, bufferSet2, bufferIndices,NUM_OF_POINTS);
            queue.finish();
        }, "Time taken for SSD (L2 Norm) calculation"
    );

    MeasureTime(
        [&] -> void
        {
            cl::copy(queue, bufferIndices, indices.begin(), indices.end());
        }, "Time taken to unload data"
    );

    ComputeChecksum(indices);

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
    for (auto const & [index, platform] : std::views::enumerate(platforms))
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
    for (auto const & [index, device] : std::views::enumerate(devices))
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
    std::string const source{FileIterator{file}, FileIterator{}};

    cl::Program program{context, source};

    try
    {
        program.build();
    }
    catch (cl::Error const & error)
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
    using namespace std::chrono;

    auto const start = high_resolution_clock::now();
    function();
    auto const stop = high_resolution_clock::now();
    auto const difference_ms = duration_cast<milliseconds>(stop - start);
    auto const time_ms = difference_ms.count();
    std::println("{}: {} ms", message, time_ms);
}

auto ComputeChecksum(std::array<std::size_t, NUM_OF_POINTS> const & indices) -> void
{
    auto const checksum = std::reduce(indices.begin(), indices.end(), 0UL, std::bit_xor{});
    std::println("Checksum : {:#x}", checksum);
}
