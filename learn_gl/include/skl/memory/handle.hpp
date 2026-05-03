#pragma once
#import "skl/memory/base.hpp"

#ifndef __cplusplus
#import "skl/memory/enum_type.hpp"
typedef struct {
    uint32_t ID;
    ResType type;
} CHandle;
#else

NAMESPACE_MEMORY_BEGIN
template<typename Tag>
struct Handle {
    uint32_t index = 0;
    uint32_t generation = 0;

    explicit operator bool() const noexcept { return index != 0; }
    bool operator==(const Handle &) const = default;
    bool operator!=(const Handle &) const = default;
};
NAMESPACE_MEMORY_END
#endif
