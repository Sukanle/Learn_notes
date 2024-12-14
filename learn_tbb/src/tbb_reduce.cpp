#include <cmath>
#include <fmt/format.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_reduce.h>
#include <vector>

float add_sin(const tbb::blocked_range<size_t>& r, float local_res)
{
    for (size_t i = r.begin(); i != r.end(); i++) {
        local_res += std::sin(i);
    }
    return local_res;
}

float add(float x, float y) { return x + y; }

int main()
{
    size_t N = 1 << 26; // 2^26 64M
    float res = tbb::parallel_reduce(tbb::blocked_range<size_t>(0, N), 0.0f, add_sin, add);

    float deterministic_res
        = tbb::parallel_deterministic_reduce(tbb::blocked_range<size_t>(0, N), 0.0f, add_sin, add);

    fmt::print("res = {}\n", res);
    fmt::print("deterministic_res = {}\n", deterministic_res);

    return 0;
}
