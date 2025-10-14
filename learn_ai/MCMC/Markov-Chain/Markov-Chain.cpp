#include <iostream>
#include <vector>
#include <mdspan>
#include <format>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>

template <typename T>
using matrix_view = std::mdspan<T, std::extents<size_t, std::dynamic_extent, std::dynamic_extent>>;

template <typename T>
void tbb_matrix_multiply(matrix_view<T> A, matrix_view<T> B, matrix_view<T> C) {
    const size_t n = A.extent(0);
    
    tbb::parallel_for(tbb::blocked_range2d<size_t>(0, n, 0, n),
        [&](const tbb::blocked_range2d<size_t>& r) {
            for(size_t i = r.rows().begin(); i < r.rows().end(); ++i) {
                for(size_t j = r.cols().begin(); j < r.cols().end(); ++j) {
                    T sum = 0;
                    for(size_t k = 0; k < n; ++k) {
                        sum += A[i, k] * B[k, j];
                    }
                    C[i, j] = sum;
                }
            }
        }
    );
}

void print_matrix(matrix_view<double> mat) {
    const size_t n = mat.extent(0);
    for(size_t i = 0; i < n; ++i) {
        for(size_t j = 0; j < n; ++j) {
            std::cout << std::format("{:8.8f} ", mat[i, j]);
        }
        std::cout << std::endl;
    }
}

int main() {
    constexpr size_t n = 3;
    std::vector<double> data = {
        0.9, 0.075, 0.025,
        0.15, 0.8, 0.05,
        0.25, 0.25, 0.5
    };
    std::vector<double> temp(n * n);

    matrix_view<double> matrix(data.data(), n, n);
    matrix_view<double> result(temp.data(), n, n);

    for(int i = 0; i < 10; ++i) {
        std::fill(temp.begin(), temp.end(), 0.0);
        tbb_matrix_multiply(matrix, matrix, result);
        
        std::swap(data, temp);
        matrix = matrix_view<double>(data.data(), n, n);
        result = matrix_view<double>(temp.data(), n, n);

        std::cout << std::format("Current round: {}\n", i+1);
        print_matrix(matrix);
        std::cout<<std::endl;
    }

    return 0;
}
