#include <iostream>
#include <chrono>
#include <random>
#include <algorithm>
#include <ranges>
#include <thread>
#include <functional>


enum Constants
{
    NUM_OF_POINTS = 10'000UL,
    SEED = 0xDEADBEEF42UL,
};


class Descriptor
{
public:
    Descriptor() noexcept
    {
        std::generate(features, features + DIMENSIONS, generator);
    }

    static float getL1Norm(Descriptor const & lhs, Descriptor const & rhs) noexcept
    {
        float sum = 0;

        for (auto const & [left, right]: std::views::zip(lhs.features, rhs.features))
        {
            sum += std::abs(left - right);
        }

        return sum;
    }

    static float getL2Norm(Descriptor const & lhs, Descriptor const & rhs) noexcept
    {
        float sum = 0;
        float diff = 0;

        for (auto const & [left, right]: std::views::zip(lhs.features, rhs.features))
        {
            diff = left - right;
            sum += diff * diff;
        }

        return std::sqrt(sum);
    }

private:
    static constexpr size_t DIMENSIONS = 128;

    static std::mt19937 randomEngine;
    static std::uniform_real_distribution<float> randomDistribution;
    static std::function<float()> generator;

    float features[DIMENSIONS];
};

std::mt19937 Descriptor::randomEngine{SEED};
std::uniform_real_distribution<float> Descriptor::randomDistribution{0.0, 1.0};
std::function<float()> Descriptor::generator = []() -> float { return randomDistribution(randomEngine); };


static Descriptor set1[NUM_OF_POINTS];
static Descriptor set2[NUM_OF_POINTS];

static size_t indicesL1[NUM_OF_POINTS];
static size_t indicesL2[NUM_OF_POINTS];

auto inline TestSpeed(std::function<void()> const & function, std::string_view const message) noexcept -> void;

auto inline CompareL1() noexcept -> void;
auto inline CompareL2() noexcept -> void;

auto inline Cooldown(std::chrono::seconds const & seconds = std::chrono::seconds{5}) -> void;


int main()
{
    std::cout << "Starting Comparing L1 Norm\n";
    TestSpeed(CompareL1, "CompareL1");

    Cooldown();

    std::cout << "Starting Comparing L2 Norm\n";
    TestSpeed(CompareL2, "CompareL2");

    return 0;
}

auto inline TestSpeed(std::function<void()> const & function, std::string_view const message) noexcept -> void
{
    auto const start = std::chrono::high_resolution_clock::now();
    function();
    auto const stop = std::chrono::high_resolution_clock::now();

    auto const difference_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    auto const time_ms = difference_ms.count();

    std::cout << "Time taken for " << message << " : " << time_ms << " ms\n";
}


auto inline CompareL1() noexcept -> void
{
    float minDistance{0.0};
    float currentDistance{0.0};

    for (int idx1 = 0; idx1 < NUM_OF_POINTS; ++idx1)
    {
        minDistance = std::numeric_limits<float>::max();

        for (int idx2 = 0; idx2 < NUM_OF_POINTS; ++idx2)
        {
            currentDistance = Descriptor::getL1Norm(set1[idx1], set2[idx2]);

            if (currentDistance < minDistance)
            {
                minDistance = currentDistance;
                indicesL1[idx1] = idx2;
            }
        }
    }
}

auto inline CompareL2() noexcept -> void
{
    float minDistance{0.0};
    float currentDistance{0.0};

    for (int idx1 = 0; idx1 < NUM_OF_POINTS; ++idx1)
    {
        minDistance = std::numeric_limits<float>::max();

        for (int idx2 = 0; idx2 < NUM_OF_POINTS; ++idx2)
        {
            currentDistance = Descriptor::getL2Norm(set1[idx1], set2[idx2]);

            if (currentDistance < minDistance)
            {
                minDistance = currentDistance;
                indicesL2[idx1] = idx2;
            }
        }
    }
}

auto inline Cooldown(std::chrono::seconds const & seconds) -> void
{
    std::this_thread::sleep_for(seconds);
}

