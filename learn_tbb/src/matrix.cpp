#include <benchmark/benchmark.h>
#include <chrono>
#include <cstring>
#include <ctime>
#include <fmt/format.h>

constexpr int N = 1000;
#define martrix_for(x, y, z)                                                                       \
    for (x = 0; x < N; x++)                                                                        \
        for (y = 0; y < N; y++)                                                                    \
            for (z = 0; z < N; z++)                                                                \
    c[i][j] += a[i][k] * b[k][j]

#define martrix_set()                                                                              \
    for (int i = 0; i < N; i++)                                                                    \
        for (int j = 0; j < N; j++) {                                                              \
            a[i][j] = float(rand()) / RAND_MAX;                                                    \
            b[i][j] = float(rand()) / RAND_MAX;                                                    \
        }
#define martrix_for_test(i, j, k, str)                                                             \
    /*martrix_set(); */                                                                            \
    /*start = std::chrono::system_clock::now(); */                                                 \
    martrix_for(i, j, k);                                                                          \
    /*end = std::chrono::system_clock::now(); */                                                   \
    /* printf(str "Time: %ld\n",                                                                   \
     * std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());                \
     * */ \
    memset(c, 0, sizeof(c));

int a[N][N] = { 0 };
int b[N][N] = { 0 };
int c[N][N] = { 0 };
int i, j, k;
std::chrono::time_point<std::chrono::system_clock> start, end;


void BM_matrix_ijk(benchmark::State& bm)
{
 martrix_set();
    i = j = k = 0;
    for (auto _ : bm) {
        martrix_for_test(i, j, k, "ijk");
    }
    return;
}
void BM_matrix_ikj(benchmark::State& bm)
{
 martrix_set();
    i = j = k = 0;
    for (auto _ : bm) {
        martrix_for_test(i, j, k, "ikj");
    }
    return;
}
void BM_matrix_jik(benchmark::State& bm)
{
 martrix_set();
    i = j = k = 0;
    for (auto _ : bm) {
        martrix_for_test(i, j, k, "jik");
    }
    return;
}
void BM_matrix_jki(benchmark::State& bm)
{
 martrix_set();
    i = j = k = 0;
    for (auto _ : bm) {
        martrix_for_test(i, j, k, "jki");
    }
    return;
}
void BM_matrix_kij(benchmark::State& bm)
{
 martrix_set();
    i = j = k = 0;
    for (auto _ : bm) {
        martrix_for_test(i, j, k, "kij");
    }
    return;
}
void BM_matrix_kji(benchmark::State& bm)
{
 martrix_set();
    i = j = k = 0;
    for (auto _ : bm) {
        martrix_for_test(i, j, k, "kji");
    }
    return;
}

BENCHMARK(BM_matrix_ijk);
BENCHMARK(BM_matrix_ikj);
BENCHMARK(BM_matrix_jki);
BENCHMARK(BM_matrix_jik);
BENCHMARK(BM_matrix_kij);
BENCHMARK(BM_matrix_kji);

BENCHMARK_MAIN();
