#pragma once

#include <stddef.h>
#include <stdint.h>
namespace skl {
inline size_t aligned(uint8_t aligned_size, size_t n) {
    if (n == 0) return aligned_size;                       // 至少分配一个对齐块
    return (n + aligned_size - 1) & ~(aligned_size - 1);   // 向上对齐
}

inline size_t aligned8(size_t n) {
    return aligned(8, n);
}
inline size_t aligned16(size_t n) {
    return aligned(16, n);
}
inline size_t aligned32(size_t n) {
    return aligned(32, n);
}
inline size_t aligned64(size_t n) {
    return aligned(64, n);
}

}   // namespace skl
