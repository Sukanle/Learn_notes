#include <benchmark/benchmark.h>
#include <cstddef>
#include <gtest/gtest.h>
#include <immintrin.h>
#include <memory.h>
#include <random>

// BEGIN CODE
void saxpy(float a, const float* __restrict x, const float* __restrict y,
    float* __restrict z, size_t n) {
    for(size_t i =0; i<n;i+=4){
        __m128 xi= _mm_loadu_ps(&x[i]);
        __m128 yi= _mm_loadu_ps(&y[i]);
        __m128 zi= a * xi + yi;
        _mm_storeu_ps(&z[i], zi);
    }
}
// END CODE

static void bench(benchmark::State &s) {
    const auto n = size_t(8192);
    auto a = 0.5f;
    auto x = std::vector<float>(n);
    auto y = std::vector<float>(n);
    auto z = std::vector<float>(n);
    auto z2 = std::vector<float>(n);
    std::generate(x.begin(), x.end(), [uni = std::uniform_real_distribution<float>(), rng = std::mt19937()] () mutable { return uni(rng); });
    std::generate(y.begin(), y.end(), [uni = std::uniform_real_distribution<float>(), rng = std::mt19937()] () mutable { return uni(rng); });
    for (auto _: s) {
        saxpy(a, x.data(), y.data(), z.data(), n);
        benchmark::DoNotOptimize(x);
        benchmark::DoNotOptimize(y);
    }
    s.SetItemsProcessed(n * s.iterations());
}
BENCHMARK(bench);

void scalar_saxpy(float a, float const *x, float const *y, float *z, size_t n) {
    for (size_t i = 0; i < n; i++) {
        z[i] = a * x[i] + y[i];
    }
}

TEST(MySuite, MyTest) {
    const auto n = size_t(8193);
    auto a = 0.5f;
    auto x = std::vector<float>(n);
    auto y = std::vector<float>(n);
    auto z = std::vector<float>(n);
    auto z2 = std::vector<float>(n);
    std::generate(x.begin(), x.end(), [uni = std::uniform_real_distribution<float>(), rng = std::mt19937()] () mutable { return uni(rng); });
    std::generate(y.begin(), y.end(), [uni = std::uniform_real_distribution<float>(), rng = std::mt19937()] () mutable { return uni(rng); });
    saxpy(a, x.data(), y.data(), z.data(), n);
    scalar_saxpy(a, x.data(), y.data(), z2.data(), n);
    for (size_t i = 0; i < n; i++) {
        EXPECT_EQ(z[i], z2[i]);
        if (z[i] != z2[i]) break;
    }
}
