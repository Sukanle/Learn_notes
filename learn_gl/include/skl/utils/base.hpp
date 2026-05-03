#pragma once

#include "skl/base.hpp"
#include <type_traits>

#define NAMESPACE_UTILS_BEGIN \
    NAMESPACE_SKL_BEGIN       \
        namespace utils {
#define NAMESPACE_UTILS_END \
    NAMESPACE_SKL_END       \
    }

NAMESPACE_UTILS_BEGIN
// NOLINTBEGIN
template<typename offset_t, typename = std::enable_if_t<std::is_trivially_copyable_v<offset_t>>>
constexpr const void *BufferOffset(size_t offset) {
    return (const void *)(sizeof(offset_t) * offset);
}
// NOLINTEND
NAMESPACE_UTILS_END
