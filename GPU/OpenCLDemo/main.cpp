#include <iostream>
#include <format>
#include <ranges>
#include <vector>
#include <fstream>

#include <CL/opencl.hpp>

#define SIZE    ( 10 )


auto main() -> int
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

    cl::Program::Sources sources;

    std::ifstream file{"../add.cl"};

    if (!file.is_open())
    {
        std::cerr << "Failed to open OpenCL source file.\n";
        return EXIT_FAILURE;
    }

    using FileIterator = std::istreambuf_iterator<char>;
    std::string source{FileIterator{file}, FileIterator{}};

    sources.emplace_back(source.data(), source.size());

    cl::Context context(device);
    cl::Program program{context, sources};
    if (program.build(device) != CL_SUCCESS)
    {
        std::cerr << "Failed to build program.\n";
        return EXIT_FAILURE;
    }

    cl::Buffer bufferA{context, CL_MEM_READ_WRITE, SIZE * sizeof(float)};
    cl::Buffer bufferB{context, CL_MEM_READ_WRITE, SIZE * sizeof(float)};
    cl::Buffer bufferC{context, CL_MEM_READ_WRITE, SIZE * sizeof(float)};

    float a[SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    float b[SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    float c[SIZE] = {0};

    cl::CommandQueue queue{context, device};

    queue.enqueueWriteBuffer(bufferA, CL_TRUE, 0, SIZE * sizeof(float), a);
    queue.enqueueWriteBuffer(bufferB, CL_TRUE, 0, SIZE * sizeof(float), b);

    cl::Kernel kernel_add{program, "add"};
    kernel_add.setArg(0, bufferA);
    kernel_add.setArg(1, bufferB);
    kernel_add.setArg(2, bufferC);

    queue.enqueueNDRangeKernel(kernel_add, cl::NullRange, cl::NDRange(SIZE), cl::NullRange);
    queue.finish();

    queue.enqueueReadBuffer(bufferC, CL_TRUE, 0, SIZE * sizeof(float), c);

    std::cout << "Result:\n";
    for (auto const & [index, value]: std::views::enumerate(c))
    {
        std::cout << std::format("c[{}] = a[{}] + b[{}] = {} + {} = {}\n", index, index, index, a[index], b[index], value);
    }

    return 0;
}