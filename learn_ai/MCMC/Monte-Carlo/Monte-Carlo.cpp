#include <charconv>
#include <cmath>
#include <fmt/format.h>
#include <random>
#include <vector>
#include <numeric>
using XYpoint = std::pair<double, double>;

double length_line(const XYpoint& point, double circle) {
    double rel_x = point.first - circle;
    double rel_y = point.second - circle;
    return std::hypot(rel_x, rel_y);
}

int main(int argc, char* argv[]) {
    // 参数解析与验证
    if (argc < 3 || argc > 4) {
        fmt::print("Usage: {} <Square> <Samples> [Iterations=5]\n", argv[0]);
        fmt::print("Example: {} 20 100000 10\n", argv[0]);
        return EXIT_FAILURE;
    }

    int square = 0, sample = 0, iterations = 5;
    auto [ptr1, ec1] = std::from_chars(argv[1], argv[1] + strlen(argv[1]), square);
    auto [ptr2, ec2] = std::from_chars(argv[2], argv[2] + strlen(argv[2]), sample);
    
    if (argc == 4) {
        auto [ptr3, ec3] = std::from_chars(argv[3], argv[3] + strlen(argv[3]), iterations);
        if (ec3 != std::errc()) iterations = 5;
    }

    if (ec1 != std::errc() || ec2 != std::errc() || 
        square <= 0 || sample <= 0 || iterations <= 0) {
        fmt::print("Invalid parameters\n");
        return EXIT_FAILURE;
    }

    const double radius = square / 2.0;
    const double real_area = M_PI * radius * radius;
    const double square_area = square * square;
    
    // 存储每次迭代结果
    std::vector<double> predictions;
    predictions.reserve(iterations);

    std::random_device rd;
    
    // 执行多次蒙特卡洛模拟
    for (int iter = 0; iter < iterations; ++iter) {
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist(0.0, square);
        
        int inside = 0;
        for (int i = 0; i < sample; ++i) {
            XYpoint point = {dist(gen), dist(gen)};
            if (length_line(point, radius) <= radius) {
                ++inside;
            }
        }
        
        double predicted = (inside / static_cast<double>(sample)) * square_area;
        predictions.push_back(predicted);
        
        fmt::print("Iter {:2} => Predicted: {:.2f} | Deviation:: {:+.2f}\n", 
                   iter+1, predicted, real_area - predicted);
    }

    // 统计计算结果
    const double mean = std::accumulate(predictions.begin(), predictions.end(), 0.0)
                      / iterations;
    
    double variance = 0.0;
    for (double pred : predictions) {
        variance += std::pow(pred - mean, 2);
    }
    variance /= iterations;
    
    const double std_dev = std::sqrt(variance);

    // 最终输出
    fmt::print("\n===== Final Results =====\n");
    fmt::print("Real area:      {:.4f}\n", real_area);
    fmt::print("Average pred:   {:.4f} (±{:.4f})\n", mean, std_dev);
    fmt::print("Min pred:       {:.4f}\n", *std::min_element(predictions.begin(), predictions.end()));
    fmt::print("Max pred:       {:.4f}\n", *std::max_element(predictions.begin(), predictions.end()));
    fmt::print("Deviation:      {:+.4f}\n", real_area - mean);
    fmt::print("Relative Deviation: {:.4f}\n", std::abs(real_area - mean) / real_area);

    return EXIT_SUCCESS;
}
