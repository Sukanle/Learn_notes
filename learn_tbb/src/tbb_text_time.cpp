#include <vector>
#include <cmath>

#include <fmt/format.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <benchmark/benchmark.h>
// #include <tbb/tick_count.h>
// #define TICK(x) auto bench_##x = tbb::tick_count::now();
// #define TOCK(x)                                                                                    \
//     fmt::print("Elapsed time in " #x ": {:f}\n", (tbb::tick_count::now() - bench_##x).seconds());

constexpr const size_t n = 1 << 27;
std::vector<float> a(n);

void BM_for(benchmark::State& bm)
{

    for (auto _ : bm) {
        // TICK(for_tbb);
        // fill a with sin(i)
        tbb::parallel_for(tbb::blocked_range<size_t>(0, n), [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); i++) {
                a[i] = std::sin(i);
            }
        });
        // TOCK(for_tbb);
    }
    return;
}
BENCHMARK(BM_for);

void BM_reduce(benchmark::State& bm)
{
    for (auto _ : bm) {
        // TICK(reduce_tbb);
        // calculate sum of a
        float res_tbb = tbb::parallel_reduce(
            tbb::blocked_range<size_t>(0, n), (float)0,
            [&](tbb::blocked_range<size_t> r, float local_res) {
                for (size_t i = r.begin(); i < r.end(); i++) {
                    local_res += a[i];
                }
                return local_res;
            },
            [](float x, float y) { return x + y; });
        // TOCK(reduce_tbb);
        benchmark::DoNotOptimize(res_tbb);
    }
    return;
}
BENCHMARK(BM_reduce);

BENCHMARK_MAIN();
