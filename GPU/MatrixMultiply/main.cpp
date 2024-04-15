#include <iostream>
#include <format>
#include <ranges>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <numeric>

#include <CL/opencl.hpp>

#define SEED         ( 0xDEADBEEF42UL )

#define M            ( 10'016UL )
#define N            ( 10'016UL )
#define K            ( 10'016UL )
#define SIZE_A       ( M * K )
#define SIZE_B       ( K * N )
#define SIZE_C       ( M * N )
#define STORAGE_A    ( SIZE_A * sizeof(float) )
#define STORAGE_B    ( SIZE_B * sizeof(float) )
#define STORAGE_C    ( SIZE_C * sizeof(float) )

#define TILE_SIZE    ( 32 )


auto GetContext() -> cl::Context;
auto GetProgram(cl::Context const & context, std::string_view const filename) -> cl::Program;

auto MeasureTime(std::function<void(void)> const & function, std::string_view const message) -> void;


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
    cl::EnqueueArgs const args{queue, cl::NDRange{M, N}, cl::NDRange{TILE_SIZE, TILE_SIZE}};

    MeasureTime([&] {
        multiplyFunctor(args, bufferA, bufferB, bufferC, M, N, K);
        queue.finish();
    }, "Time taken for matrix multiplication");

    MeasureTime([&] {
        cl::copy(queue, bufferC, result.begin(), result.end());
    }, "Time taken to unload data");

    auto const checksum = std::reduce(result.cbegin(), result.cend(), 0.0F, [] (auto const & lhs, auto const & rhs) -> auto { return (lhs + rhs) / 100.0F; });

    std::cout << std::format("Checksum: {}\n", checksum);

    return 0;
}


auto GetContext() -> cl::Context
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