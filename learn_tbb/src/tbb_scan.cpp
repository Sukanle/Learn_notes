#include <cmath>
#include <fmt/format.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_scan.h>
#include <vector>

int main()
{
    size_t N = 1 << 26; // 2^26 64M
    std::vector<float> data(N);
    float res_tbb = tbb::parallel_scan(
        tbb::blocked_range<size_t>(0, N), 0.0f,
        [&](const tbb::blocked_range<size_t>& r, float local_res, auto is_final) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                local_res += std::sin(i);
                if (is_final) {
                    data[i] = local_res;
                }
            }
            return local_res;
        },
        [](float x, float y) { return x + y; });
    fmt::print("tbb on\n");
    fmt::print("a[n/2] = {}\n", data[N / 2]);
    fmt::print("res_tbb = {}\n", res_tbb);
    return 0;
}
