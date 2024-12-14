#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>
#include <vector>
#include <cmath>
#include <thread>
#include <fmt/format.h>
#include "ticktock.h"

int main(){
    size_t N = 1<<14;
    std::vector<float> a(N*N);
    std::vector<float> b(N*N);

    TICK(transpose);
    tbb::parallel_for(tbb::blocked_range2d<size_t>(0, N, 0, N),
        [&](const tbb::blocked_range2d<size_t>& r){
            for(size_t i=r.rows().begin(); i<r.rows().end(); i++){
                for(size_t j=r.cols().begin(); j<r.cols().end(); j++){
                    b[i*N+j] = a[j*N+i];
                }
            }
        });
    TOCK(transpose);

    return 0;
}
