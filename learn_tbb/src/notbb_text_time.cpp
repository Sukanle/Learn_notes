#include <benchmark/benchmark.h>
#include <cmath>
#include <fmt/format.h>
#include <tbb/tick_count.h>
#include <vector>
// #define TICK(x) auto bench_##x = tbb::tick_count::now();
// #define TOCK(x) \
//     fmt::print("Elapsed time in " #x ": {:f}\n", (tbb::tick_count::now() - bench_##x).seconds());

constexpr const size_t n = 1 << 27;
std::vector<float> a(n);

void BM_for(benchmark::State& bm)
{
    for (auto _ : bm) {
        // TICK(for);
        // fill a with sin(i)
        for (size_t i = 0; i < a.size(); i++) {
            a[i] = std::sin(i);
        }
        // TOCK(for);
    }

    return;
}
BENCHMARK(BM_for);

void BM_reduce(benchmark::State& bm)
{
    for (auto _ : bm) {
        // TICK(reduce);
        // calculate sum of a
        float res = 0;
        for (size_t i = 0; i < a.size(); i++) {
            res += a[i];
        }
        // TOCK(reduce);
        benchmark::DoNotOptimize(res);
    }

    return;
}
BENCHMARK(BM_reduce);

BENCHMARK_MAIN();
