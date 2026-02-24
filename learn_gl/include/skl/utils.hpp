#pragma once

#include <type_traits>

namespace skl {
// NOLINTBEGIN
template<typename offset_t, typename = std::enable_if<std::is_arithmetic<offset_t>::value>::type>
constexpr const void* BufferOffset(size_t offset) {
    return (const void*)(sizeof(offset_t) * offset);
}
// NOLINTEND
}   // namespace skl
