#include <iostream>
#include <format>
#include <ranges>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <fstream>

#include <CL/opencl.hpp>

#define SEED         ( 0xDEADBEEF42UL )


#define SIZE       ( 100'000'000 )
#define STORAGE    ( SIZE * sizeof(float) )


auto GetContext(void) -> cl::Context;
auto GetProgramme(cl::Context const & context, std::string_view const filename) -> cl::Program;

auto MeasureTime(std::function<void(void)> const & function, std::string_view const message) -> void;


using namespace std::chrono;


auto main() -> int
{
    cl::Context const context = GetContext();
    cl::Program const program = GetProgramme(context, "../add.cl");

    std::mt19937 randomEngine{SEED};
    std::uniform_real_distribution<float> randomDistribution{0.0, 1.0};
    auto generator = [&]() -> float { return randomDistribution(randomEngine); };

    std::vector<float> vec1(SIZE);
    std::vector<float> vec2(SIZE);
    std::vector<float> result(SIZE);

    std::generate(vec1.begin(), vec1.end(), generator);
    std::generate(vec2.begin(), vec2.end(), generator);

    cl::Buffer bufferA{context, CL_MEM_READ_WRITE, STORAGE};
    cl::Buffer bufferB{context, CL_MEM_READ_WRITE, STORAGE};
    cl::Buffer bufferC{context, CL_MEM_WRITE_ONLY, STORAGE};

    cl::CommandQueue queue{context};

    MeasureTime([&] {
        cl::copy(queue, vec1.begin(), vec1.end(), bufferA);
        cl::copy(queue, vec2.begin(), vec2.end(), bufferB);
    }, "Time taken to load data");

    cl::KernelFunctor<cl::Buffer, cl::Buffer, cl::Buffer> multiplyFunctor{program, "add"};

    MeasureTime([&] {
        multiplyFunctor(cl::EnqueueArgs{queue, cl::NDRange{SIZE}}, bufferA, bufferB, bufferC);
        queue.finish();
    }, "Time taken for vector addition");

    MeasureTime([&] {
        cl::copy(queue, bufferC, result.begin(), result.end());
    }, "Time taken to unload data");

    return 0;
}


auto GetContext(void) -> cl::Context
{
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty())
    {
        std::cerr << "No OpenCL platforms found. Check your OpenCL installation.\n";
        return EXIT_FAILURE;
    }

    std::cout << std::format("Found {} OpenCL platform(s):\n", platforms.size());
    for (auto const & [index, platform]: std::views::enumerate(platforms))
    {
        std::cout << std::format("Platform {}: {}\n", index, platform.getInfo<CL_PLATFORM_NAME>());
    }

    cl::Platform const platform = platforms.front();

    std::cout << std::format("\nUsing platform: {}\n", platform.getInfo<CL_PLATFORM_NAME>());

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

    if (devices.empty())
    {
        std::cerr << "No OpenCL devices found. Check your OpenCL installation.\n";
        return EXIT_FAILURE;
    }

    std::cout << std::format("\nFound {} OpenCL device(s):\n", devices.size());
    for (auto const & [index, device]: std::views::enumerate(devices))
    {
        std::cout << std::format("Device {}: {}\n", index, device.getInfo<CL_DEVICE_NAME>());
    }

    cl::Device const device = devices.front();
    std::cout << std::format("\nUsing device: {}\n", device.getInfo<CL_DEVICE_NAME>());

    return cl::Context{device};
}

auto GetProgramme(cl::Context const & context, std::string_view const filename) -> cl::Program
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
            std::cerr << "Build log:\n";
            std::cerr << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(context.getInfo<CL_CONTEXT_DEVICES>().front());
        }
        
        throw;
    }

    return program;
}


auto MeasureTime(std::function<void(void)> const & function, std::string_view const message) -> void
{
    auto const start = high_resolution_clock::now();
    function();
    auto const stop = high_resolution_clock::now();
    auto const difference_ms = duration_cast<milliseconds>(stop - start);
    auto const time_ms = difference_ms.count();
    std::cout << std::format("{}: {} ms\n", message, time_ms);
}