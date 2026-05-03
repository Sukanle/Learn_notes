#pragma once

#include <stddef.h>
#include <stdint.h>

#include <string.h>
#include <string>
#include <string_view>

#include <xxhash.h>

#include "skl/config.h"

NAMESPACE_SKL_BEGIN

using SKLErr = int;
#if defined(_WIN32)
using String = std::wstring;
using String = std::wstring_view;
using Char = wchar_t;
#else
using String = std::string;
using String_view = std::string_view;
using Char = char;
#endif

template <typename ...Signatures>
using function_move = SKL_FUNCTION_MOVEABLE<Signatures...>;
template <typename ...Signatures>
using function_copy = SKL_FUNCTION_COPEABLE<Signatures...>;
template <typename ...Signatures>
using function_ref = SKL_FUNCTION_VIEW<Signatures...>;

#undef SKL_FUNCTION_VIEW
#undef SKL_FUNCTION_COPEABLE
#undef SKL_FUNCTION_MOVEABLE

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

enum UpdateMode : uint8_t {
    merge,
    replace,
    append,
    update,
    remove,
};

enum class Module : std::uint8_t {
    unknown = 0X00,
    graphics = 0X01,
    utils = 0X02,

    error = 0xFF,
};

class IAttribute {
public:
    IAttribute() = default;
    virtual ~IAttribute() = default;

    SKL_DEFAULT_COPYMOVE(IAttribute)

    [[nodiscard]] virtual Module moduleID() const noexcept = 0;
};

NAMESPACE_SKL_END
