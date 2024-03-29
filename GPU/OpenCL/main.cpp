#include <iostream>
#include <format>
#include <ranges>
#include <vector>

#include <CL/opencl.hpp>

auto main() -> int
{
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty())
    {
        std::cerr << "No OpenCL platforms found. Check your OpenCL installation." << std::endl;
    }

    std::cout << std::format("Found {} OpenCL platform(s):\n", platforms.size());
    for (auto const & [index, platform] : std::views::enumerate(platforms))
    {
        std::cout << std::format("Platform {}: {}\n", index, platform.getInfo<CL_PLATFORM_NAME>());
    }

    cl::Platform const platform = platforms.front();

    std::cout << std::format("\nUsing platform: {}\n", platform.getInfo<CL_PLATFORM_NAME>());

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

    if (devices.empty())
    {
        std::cerr << "No OpenCL devices found. Check your OpenCL installation." << std::endl;
    }

    std::cout << std::format("\nFound {} OpenCL device(s):\n", devices.size());
    for (auto const & [index, device] : std::views::enumerate(devices))
    {
        std::cout << std::format("Device {}: {}\n", index, device.getInfo<CL_DEVICE_NAME>());
    }

    cl::Device const device = devices.front();

    std::cout << std::format("\nUsing device: {}\n", device.getInfo<CL_DEVICE_NAME>());

    cl::Context context{device};

    return 0;
}