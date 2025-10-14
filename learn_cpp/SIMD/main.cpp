#include <benchmark/benchmark.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <random>
#include <memory>
#include <cstddef>
#include <immintrin.h>

#define MAX_SIZE 1024

[[gnu::optimize("O2")]] void saxpy128(float a, float const *x, float const *y, float *z, size_t n) {
    size_t i;
    __m128 av = _mm_set1_ps(a);
    for (i = 0; i + 7 < n; i += 8) {
        __m128 xi = _mm_loadu_ps(&x[i]);
        __m128 xi2 = _mm_loadu_ps(&x[i + 4]);
        __m128 yi = _mm_loadu_ps(&y[i]);
        __m128 yi2 = _mm_loadu_ps(&y[i + 4]);
        __m128 zi = _mm_add_ps(_mm_mul_ps(av, xi), yi);
        __m128 zi2 = _mm_add_ps(_mm_mul_ps(av, xi2), yi2);
        _mm_storeu_ps(&z[i], zi);
        _mm_storeu_ps(&z[i + 4], zi2);
    }
    for (; i < n; i++) {
        z[i] = a * x[i] + y[i];
    }
}

// BEGIN CODE
[[gnu::optimize("O2")]] void saxpy256(float a, float const *x, float const *y, float *z, size_t n) {
    size_t i;
    __m256 av = _mm256_set1_ps(a);

    for (i = 0; i + 7 < n; i += 8) {
        __m256 xi = _mm256_loadu_ps(&x[i]);
        __m256 yi = _mm256_loadu_ps(&y[i]);
        __m256 zi = _mm256_add_ps(_mm256_mul_ps(av, xi), yi);
        _mm256_storeu_ps(&z[i], zi);
    }
    if(n >= 4)
        return saxpy128(a, x, y, z, n);
    for (; i < n; i++) {
        z[i] = a * x[i] + y[i];
    }
}
// END CODE

static void bench_128(benchmark::State &s) {
    const size_t n = 8192;
    auto a = 0.5f;
    auto x = std::vector<float>(n);
    auto y = std::vector<float>(n);
    auto z = std::vector<float>(n);
    std::generate(x.begin(), x.end(), [uni = std::uniform_real_distribution<float>(), rng = std::mt19937()] () mutable { return uni(rng); });
    std::generate(y.begin(), y.end(), [uni = std::uniform_real_distribution<float>(), rng = std::mt19937()] () mutable { return uni(rng); });
    for (auto _: s) {
        saxpy128(a, x.data(), y.data(), z.data(), n);
        benchmark::DoNotOptimize(x);
        benchmark::DoNotOptimize(y);
    }
    s.SetItemsProcessed(n * s.iterations());
}
BENCHMARK(bench_128);

static void bench_256(benchmark::State &s) {
    const size_t n = MAX_SIZE;
    auto a = 0.5f;
    auto x = std::vector<float>(n);
    auto y = std::vector<float>(n);
    auto z = std::vector<float>(n);
    std::generate(x.begin(), x.end(), [uni = std::uniform_real_distribution<float>(), rng = std::mt19937()] () mutable { return uni(rng); });
    std::generate(y.begin(), y.end(), [uni = std::uniform_real_distribution<float>(), rng = std::mt19937()] () mutable { return uni(rng); });
    for (auto _: s) {
        saxpy256(a, x.data(), y.data(), z.data(), n);
        benchmark::DoNotOptimize(x);
        benchmark::DoNotOptimize(y);
    }
    s.SetItemsProcessed(n * s.iterations());
}
BENCHMARK(bench_256);

void scalar_saxpy(float a, float const *x, float const *y, float *z, size_t n) {
    for (size_t i = 0; i < n; i++) {
        z[i] = a * x[i] + y[i];
    }
}

TEST(MySuite, MyTest) {
    const auto n = size_t(MAX_SIZE);
    auto a = 0.5f;
    auto x = std::vector<float>(n);
    auto y = std::vector<float>(n);
    auto z = std::vector<float>(n);
    auto z1 = std::vector<float>(n);
    auto z2 = std::vector<float>(n);
    std::generate(x.begin(), x.end(), [uni = std::uniform_real_distribution<float>(), rng = std::mt19937()] () mutable { return uni(rng); });
    std::generate(y.begin(), y.end(), [uni = std::uniform_real_distribution<float>(), rng = std::mt19937()] () mutable { return uni(rng); });
    saxpy128(a, x.data(), y.data(), z.data(), n);
    saxpy256(a, x.data(), y.data(), z1.data(), n);
    scalar_saxpy(a, x.data(), y.data(), z2.data(), n);
    for (size_t i = 0; i < n; i++) {
        EXPECT_EQ(z[i], z2[i]);
        if (z[i] != z2[i]) break;
    }
}
