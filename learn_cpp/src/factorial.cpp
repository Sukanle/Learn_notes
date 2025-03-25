// #include <immintrin.h>
#include <math.h>
#include <stdint.h>

// 如果为C++编译器
#ifdef __cplusplus
#include <algorithm>
#include <benchmark/benchmark.h>
#include <random>
#include <stdexcept>
#include <vector>
// 预计算阶乘缓存
constexpr int MAX_PRECOMPUTED = 20;
alignas(64) uint64_t __factorial_cache[MAX_PRECOMPUTED + 1] = {1};

constexpr bool init_factorial_cache(uint64_t* cache = __factorial_cache) {
#pragma unroll(4)
    for (int i = 1; i <= MAX_PRECOMPUTED; i++) { cache[i] = cache[i - 1] * i; }
    return true;
}

// 优化阶乘计算
constexpr uint64_t optimized_factorial(int n,
                                       uint64_t* cache = __factorial_cache) {
    bool initialized = init_factorial_cache();
    if (n < 0) {
        throw std::invalid_argument(
            "Negative input, Function: optimized_factorial");
    }
    if (n <= MAX_PRECOMPUTED) { return cache[n]; }
    uint64_t result = cache[MAX_PRECOMPUTED];
#pragma omp parallel for reduction(* : result)
    for (int i = MAX_PRECOMPUTED + 1; i <= n; ++i) { result *= i; }
    return result;
}

// 大数阶乘计算
std::vector<uint64_t> big_factorial(int n) {
    std::vector<uint64_t> digits = {1};
    for (int i = 1; i <= n; ++i) {
        uint64_t carry = 0;
        for (auto& d : digits) {
            uint64_t temp = d * i + carry;
            d = temp % 10;
            carry = temp / 10;
        }
        while (carry) {
            digits.push_back(carry % 10);
            carry /= 10;
        }
    }
    std::reverse(digits.begin(), digits.end());
    return digits;
}

constexpr bool bigfac = true;
constexpr bool optfac = false;

template<bool return_t>
constexpr std::conditional_t<return_t, std::vector<uint64_t>, uint64_t>
    factorial(int n) {
    if constexpr (return_t) {
        return big_factorial(n);
    } else {
        return optimized_factorial(n);
    }
}

const struct {
    double S1 = 1.0 / factorial<optfac>(3);
    double S2 = 1.0 / factorial<optfac>(5);
    double S3 = 1.0 / factorial<optfac>(7);
    double S4 = 1.0 / factorial<optfac>(9);
} taylor;

uint64_t GetHighWord(double x) {
    uint64_t int_rep = *reinterpret_cast<uint64_t*>(&x);
    return static_cast<uint64_t>(int_rep >> 32);
}

double optimized_sin(double x) {
    // 周期与符号处理
    x = fmod(x, 2 * M_PI);
    if (x < 0) { x += 2 * M_PI; }
    uint64_t sign = (x > M_PI) ? 0x80000000 : 0;
    x = (x > M_PI) ? x - M_PI : x;

    // 象限缩减
    if (x > M_PI_2) { x = M_PI - x; }
    // 极小值快速返回
    uint64_t ix = GetHighWord(x) & 0x7FFFFFFF;

    if (ix < 0x3E400000) { return copysign(x, sign); }

    double z = x * x;
    double v = z * x;
    double r = taylor.S2 + z * taylor.S3;
    return copysign(x + v * (taylor.S1 + z * r), sign);
}   // ��K

// ��K

// 随机数生成
static std::vector<double> GenerateRandomNumbers(size_t n) {
    std::vector<double> numbers(n);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 2 * M_PI);

    for (auto& number : numbers) { number = dis(gen); }
    return numbers;
}

static const std::vector<double> kTestData = GenerateRandomNumbers(1'000'000);

static void BN_NormalFactorial(benchmark::State& state) {
    for (auto _ : state) {
        for (int i = 0; i < 20; i++) {
            benchmark::DoNotOptimize(factorial<optfac>(i));
        }
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BN_NormalFactorial)->Unit(benchmark::kMillisecond);

static void BN_BigFactorial(benchmark::State& state) {
    for (auto _ : state) {
        for (int i = 0; i < 20; i++) {
            benchmark::DoNotOptimize(factorial<bigfac>(i));
        }
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BN_BigFactorial)->Unit(benchmark::kMillisecond);

static void BN_OptimizedSin(benchmark::State& state) {
    for (auto _ : state) {
        for (const auto& x : kTestData) {
            benchmark::DoNotOptimize(optimized_sin(x));
        }
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BN_OptimizedSin)->Unit(benchmark::kMillisecond);

static void BN_StandardSin(benchmark::State& state) {
    for (auto _ : state) {
        for (const auto& x : kTestData) { benchmark::DoNotOptimize(sin(x)); }
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BN_StandardSin)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
#else
// C语言版本
#include <CUnit/Basic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PRECOMPUTED 20
int32_t __factorial_cache[MAX_PRECOMPUTED + 1] = {1};

bool init_factorial_cache(int32_t* cache) {
    for (int i = 1; i <= MAX_PRECOMPUTED; i++) { cache[i] = cache[i - 1] * i; }
    return true;
}

size_t optimized_factorial(int n, int32_t* cache) {
    bool initialized = init_factorial_cache(cache);
    if (n < 0) { return 0; }
    if (n <= MAX_PRECOMPUTED) { return cache[n]; }
    size_t result = cache[MAX_PRECOMPUTED];
    for (int i = MAX_PRECOMPUTED + 1; i <= n; ++i) { result *= i; }
    return result;
}

size_t multiply(int x, int res[], size_t res_size) {
    int carry = 0;
    for (int i = 0; i < res_size; i++) {
        int prod = res[i] * x + carry;
        res[i] = prod % 10;
        carry = prod / 10;
    }
    while (carry) {
        res[res_size++] = carry % 10;
        carry /= 10;
    }
    return res_size;
}

// 大数阶乘计算
// 调用者提供指针，大小不为0，使用时需保证足够大
// 大小为0，函数内部自动申请内存，不会释放，且自动扩容
// 返回为占用的内存大小（实际位数）
// 打印方式如下：
// 注意：i为size_t类型，因此不会出现负数，所以利用其减至0后，变为
// size_t类型最大的正整数大于size最大迭代范围来作为终止条件
// for (size_t i = size - 1; i <= size; i--)
//   printf("%d", digits[i]);
size_t big_factorial(int n, int32_t** result, size_t size) {
    int32_t* res = NULL;
    if (size == 0) {
        size = 64;   // 默认初始分配大小
        // result = (int32_t *)malloc(size * sizeof(int32_t));
        res = (int32_t*)malloc(size * sizeof(int32_t));
        if (!res) {
            return 0;   // 内存分配失败
        }
    }

    res[0] = 1;
    size_t res_size = 1;

    for (int x = 2; x <= n; x++) {
        res_size = multiply(x, res, res_size);
        // 动态扩容逻辑（引用自[3,7](@ref)）
        if (res_size >= size) {
            size *= 2;
            int32_t* new_res = (int32_t*)realloc(res, size * sizeof(int32_t));
            if (!new_res) {
                free(res);
                return 0;   // 内存分配失败
            }
            res = new_res;
        }
    }
    // 指针指向的地址
    *result = res;
    return res_size;
}

// 利用C语言的宏对字符串进行拼接，实现函数重载
// fn选项：big、normal
#define factorial(fn, n, ...) fn##_factorial(n, __VA_ARGS__)
// normal_factorial宏定义, 吞掉__VA_ARGS__参数
#define normal_factorial(n, ...) optimized_factorial(n, __factorial_cache)

// 测试0的阶乘
void test_zero_factorial(void) {
    CU_ASSERT_EQUAL(optimized_factorial(0, __factorial_cache), 1);
}

// 测试预计算范围内的数值
void test_precomputed_range(void) {
    for (int i = 1; i <= MAX_PRECOMPUTED; ++i) {
        CU_ASSERT_EQUAL(optimized_factorial(i, __factorial_cache),
                        __factorial_cache[i]);
    }
}

// 测试随机正常值（包含缓存内外）
void test_random_values(void) {
    srand(time(NULL));
    for (int i = 0; i < 20; ++i) {
        int n = rand() % 30;   // 限制在可验证范围内
        size_t result = optimized_factorial(n, __factorial_cache);

        // 验证结果有效性
        CU_ASSERT_TRUE(result >= (n < 2 ? 1 : n));
        if (n <= MAX_PRECOMPUTED) {
            CU_ASSERT_EQUAL(result, __factorial_cache[n]);
        }
    }
}

// 测试大数阶乘基础功能
void test_big_factorial(void) {
    int32_t* result = NULL;
    size_t size = big_factorial(10, &result, 0);
    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_EQUAL(size, 7);   // 10! = 3628800（7位数字）
    free(result);
}

// 测试错误输入
void test_negative_input(void) {
    CU_ASSERT_EQUAL(optimized_factorial(-5, __factorial_cache), 0);
}

CU_TestInfo factorial_tests[] = {{"零值测试", test_zero_factorial},
                                 {"预计算范围验证", test_precomputed_range},
                                 {"随机参数测试", test_random_values},
                                 {"大数阶乘基础", test_big_factorial},
                                 {"错误输入检测", test_negative_input},
                                 CU_TEST_INFO_NULL};

int suite_init(void) {
    return init_factorial_cache(__factorial_cache) ? 0 : -1;
}

int suite_clean(void) {
    return 0;   // 无需特殊清理
}

int main() {
    CU_SuiteInfo suites[] = {{"阶乘函数测试套件", suite_init, suite_clean, NULL,
                              NULL, factorial_tests},
                             CU_SUITE_INFO_NULL};

    if (CUE_SUCCESS != CU_initialize_registry()) { return CU_get_error(); }

    if (CUE_SUCCESS != CU_register_suites(suites)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
#endif
