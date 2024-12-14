#include <atomic>
#include <cmath>
#include <vector>

#include <tbb/parallel_for.h>
#include <dpl/iterator>

#include "pod.h"
#include "ticktock.h"

int main()
{
    size_t N = 1 << 27;
    std::vector<pod<float>> a;
    std::atomic<size_t> a_size(0);

    TICK(filter);
    a.resize(N);
    tbb::parallel_for(tbb::blocked_range<size_t>(0, N), [&](tbb::blocked_range<size_t> r) {
        std::vector<pod<float>> a_local(r.size());
        size_t a_size_local = 0;
#ifndef _ONEDPL_ITERATOR
        for(auto i=r.begin(); i!=r.end(); ++i) {
#else
        for(auto i=dpl::begin(r); i!=dpl::end(r); ++i) {
#endif
            float val = std::sin(i);
            if (val > 0)
                a_local[a_size_local++] = val;
        }
        size_t base = a_size.fetch_add(a_size_local);
        for (size_t i = 0; i < a_size_local; ++i)
            a[base + i] = a_local[i];
    });
    a.resize(a_size);
    TOCK(filter);

    return 0;
}
