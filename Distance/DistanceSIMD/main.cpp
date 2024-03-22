#include <iostream>
#include <random>
#include <algorithm>
#include <ranges>
#include <thread>
#include <functional>
#include <format>
#include <new>

#include <immintrin.h>


#define ALIGN    std::hardware_destructive_interference_size

#define REDUCE_SUM(RESULT, VECTOR)    permuted = _mm256_permute2f128_ps(VECTOR, VECTOR, 1); /* Permute the two 128-bit halves of the vector */ \
                                      sum0 = _mm256_add_ps(VECTOR, permuted);               /* Add the two opposite halves of the vector */ \
                                      sum1 = _mm256_hadd_ps(sum0, sum0);                    /* Add the two adjacent floats in each group of 2 floats */ \
                                      sum2 = _mm256_hadd_ps(sum1, sum1);                    /* Add the last two floats in the vector */ \
                                      RESULT += _mm256_cvtss_f32(sum2);                     /* Convert the result to a float and add it to the sum */

#define L1_NORM(SUM, LHS, RHS, IDX)   left = _mm256_load_ps(LHS.features + IDX);               /* Load 8 floats from lhs into a vector */ \
                                      right = _mm256_load_ps(RHS.features + IDX);              /* Load 8 floats from rhs into a vector */ \
                                      diff = _mm256_sub_ps(left, right);                       /* Subtract the two vectors */ \
                                      absDiff = _mm256_andnot_ps(_mm256_set1_ps(-0.0F), diff); /* Get the absolute value of the difference (trick to clear the sign bit) */ \
                                      REDUCE_SUM(SUM, absDiff); 

#define L2_NORM(SUM, LHS, RHS, IDX)   left = _mm256_load_ps(LHS.features + IDX);  /* Load 8 floats from lhs into a vector */ \
                                      right = _mm256_load_ps(RHS.features + IDX); /* Load 8 floats from rhs into a vector */ \
                                      diff = _mm256_sub_ps(left, right);          /* Subtract the two vectors */ \
                                      squared = _mm256_mul_ps(diff, diff);        /* Square the difference */ \
                                      REDUCE_SUM(SUM, squared);
                                    


enum Constants
{
    NUM_OF_POINTS = 10'000UL,
    SEED = 0xDEADBEEF42UL,
    FLOAT_VECTOR_SIZE = 8
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
        __m256 left, right, diff, absDiff, permuted, sum0, sum1, sum2;

        float sum{0.0};

        L1_NORM(sum, lhs, rhs, 0 * FLOAT_VECTOR_SIZE);
        L1_NORM(sum, lhs, rhs, 1 * FLOAT_VECTOR_SIZE);
        L1_NORM(sum, lhs, rhs, 2 * FLOAT_VECTOR_SIZE);
        L1_NORM(sum, lhs, rhs, 3 * FLOAT_VECTOR_SIZE);
        L1_NORM(sum, lhs, rhs, 4 * FLOAT_VECTOR_SIZE);
        L1_NORM(sum, lhs, rhs, 5 * FLOAT_VECTOR_SIZE);
        L1_NORM(sum, lhs, rhs, 6 * FLOAT_VECTOR_SIZE);
        L1_NORM(sum, lhs, rhs, 7 * FLOAT_VECTOR_SIZE);
        L1_NORM(sum, lhs, rhs, 8 * FLOAT_VECTOR_SIZE);
        L1_NORM(sum, lhs, rhs, 9 * FLOAT_VECTOR_SIZE);
        L1_NORM(sum, lhs, rhs, 10 * FLOAT_VECTOR_SIZE);
        L1_NORM(sum, lhs, rhs, 11 * FLOAT_VECTOR_SIZE);
        L1_NORM(sum, lhs, rhs, 12 * FLOAT_VECTOR_SIZE);
        L1_NORM(sum, lhs, rhs, 13 * FLOAT_VECTOR_SIZE);
        L1_NORM(sum, lhs, rhs, 14 * FLOAT_VECTOR_SIZE);
        L1_NORM(sum, lhs, rhs, 15 * FLOAT_VECTOR_SIZE);

        return sum;
    }

    static float getL2Norm(Descriptor const & lhs, Descriptor const & rhs) noexcept
    {
        __m256 left, right, diff, squared, permuted, sum0, sum1, sum2;

        float sum{0.0};

        L2_NORM(sum, lhs, rhs, 0 * FLOAT_VECTOR_SIZE);
        L2_NORM(sum, lhs, rhs, 1 * FLOAT_VECTOR_SIZE);
        L2_NORM(sum, lhs, rhs, 2 * FLOAT_VECTOR_SIZE);
        L2_NORM(sum, lhs, rhs, 3 * FLOAT_VECTOR_SIZE);
        L2_NORM(sum, lhs, rhs, 4 * FLOAT_VECTOR_SIZE);
        L2_NORM(sum, lhs, rhs, 5 * FLOAT_VECTOR_SIZE);
        L2_NORM(sum, lhs, rhs, 6 * FLOAT_VECTOR_SIZE);
        L2_NORM(sum, lhs, rhs, 7 * FLOAT_VECTOR_SIZE);
        L2_NORM(sum, lhs, rhs, 8 * FLOAT_VECTOR_SIZE);
        L2_NORM(sum, lhs, rhs, 9 * FLOAT_VECTOR_SIZE);
        L2_NORM(sum, lhs, rhs, 10 * FLOAT_VECTOR_SIZE);
        L2_NORM(sum, lhs, rhs, 11 * FLOAT_VECTOR_SIZE);
        L2_NORM(sum, lhs, rhs, 12 * FLOAT_VECTOR_SIZE);
        L2_NORM(sum, lhs, rhs, 13 * FLOAT_VECTOR_SIZE);
        L2_NORM(sum, lhs, rhs, 14 * FLOAT_VECTOR_SIZE);
        L2_NORM(sum, lhs, rhs, 15 * FLOAT_VECTOR_SIZE);

        return std::sqrt(sum);
    }

private:
    static constexpr size_t DIMENSIONS = 128;

    static std::mt19937 randomEngine;
    static std::uniform_real_distribution<float> randomDistribution;
    static std::function<float()> generator;

    float features alignas(ALIGN) [DIMENSIONS];
};

std::mt19937 Descriptor::randomEngine{SEED};
std::uniform_real_distribution<float> Descriptor::randomDistribution{0.0, 1.0};
std::function<float()> Descriptor::generator = []() -> float { return randomDistribution(randomEngine); };


static Descriptor set1 alignas(ALIGN) [NUM_OF_POINTS];
static Descriptor set2 alignas(ALIGN) [NUM_OF_POINTS];

static size_t indicesL1 alignas(ALIGN) [NUM_OF_POINTS];
static size_t indicesL2 alignas(ALIGN) [NUM_OF_POINTS];

auto inline TestSpeed(std::function<void()> const & function, std::string_view const message) noexcept -> void;
auto inline ComputeChecksum(size_t const * const indices, std::string_view const message) noexcept -> void;

auto inline CompareL1() noexcept -> void;
auto inline CompareL2() noexcept -> void;

auto inline Cooldown(std::chrono::seconds const & seconds = std::chrono::seconds{5}) -> void;


int main()
{
    std::cout << "Starting Comparing L1 Norm\n";
    TestSpeed(CompareL1, "CompareL1");

    ComputeChecksum(indicesL1, "L1 Norm");

    Cooldown();

    std::cout << "Starting Comparing L2 Norm\n";
    TestSpeed(CompareL2, "CompareL2");

    ComputeChecksum(indicesL2, "L2 Norm");

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

auto inline ComputeChecksum(size_t const * const indices, std::string_view const message) noexcept -> void
{
    auto const checksum = std::reduce(indices, indices + NUM_OF_POINTS, 0UL, std::bit_xor<>());
    std::cout << std::format("Checksum for {} : {:#x}\n", message, checksum);
}


auto inline CompareL1() noexcept -> void
{
    float minDistance{0.0};
    float currentDistance{0.0};

    for (size_t idx1 = 0; idx1 < NUM_OF_POINTS; ++idx1)
    {
        minDistance = std::numeric_limits<float>::max();

        for (size_t idx2 = 0; idx2 < NUM_OF_POINTS; ++idx2)
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

    for (size_t idx1 = 0; idx1 < NUM_OF_POINTS; ++idx1)
    {
        minDistance = std::numeric_limits<float>::max();

        for (size_t idx2 = 0; idx2 < NUM_OF_POINTS; ++idx2)
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