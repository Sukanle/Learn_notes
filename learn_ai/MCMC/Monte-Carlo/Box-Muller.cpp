#include <iostream>
#include <random>
#include <cmath>
#include <vector>
#include <fmt/format.h>


std::pair<double, double> box_muller(double u1, double u2) {
    const double pi = acos(-1.0);
    double factor = sqrt(-2 * log(u1));
    double angle = 2 * pi * u2;
    return {factor * cos(angle), factor * sin(angle)};
}

std::vector<double> generate_normal(int n) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    std::vector<double> samples;
    samples.reserve(2 * n);
    double u1,u2;
    for (int i = 0; i < n; ++i) {
        u1 = dist(gen);
        u2 = dist(gen);
        auto [z0, z1] = box_muller(u1, u2);
        samples.push_back(z0);
        samples.push_back(z1);
    }

    return samples;
}

int main() {
    auto samples = generate_normal(5000);
    
    // 输出前5个样本
    fmt::print("生成样本示例（前5个）: ");
    for (int i = 0; i < 5; ++i) {
        fmt::print("{:.8f} ",samples[i]);
    }
    fmt::print("\n");

    // 计算均值和标准差
    float sum = 0, sq_sum = 0;
    for (float x : samples) {
        sum += x;
        sq_sum += x * x;
    }
    float mean = sum / samples.size();
    float stdev = sqrt(sq_sum / samples.size() - mean * mean);

    fmt::print("均值: {:.4f}, 标准差: {:.4f}\n", mean, stdev);

    return 0;
}
