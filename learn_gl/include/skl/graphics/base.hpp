#pragma once

#include <stdint.h>
#include <system_error>

#include "skl/base.hpp"

#ifndef NAMESPACE_GRAPHICS_BEGIN
#  define NAMESPACE_GRAPHICS_BEGIN \
      NAMESPACE_SKL_BEGIN          \
      namespace graphics {
#  define NAMESPACE_GRAPHICS_END \
      NAMESPACE_SKL_END          \
      }
#endif

NAMESPACE_GRAPHICS_BEGIN

enum class Backend : uint8_t {
    unknown,
    opengl,
    vulkan,
};

class IGraphicsAPI : virtual public IAttribute {
public:
    IGraphicsAPI() = default;
    ~IGraphicsAPI() override = default;

    SKL_DEFAULT_COPYMOVE(IGraphicsAPI);

    [[nodiscard]] Module moduleID() const noexcept override { return Module::graphics; }
    [[nodiscard]] virtual Backend backend() const noexcept = 0;
    // virtual bool initialize(std::error_code &ec) noexcept = 0;
    // virtual void shutdown() noexcept = 0;
};

NAMESPACE_GRAPHICS_END
