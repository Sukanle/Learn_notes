#pragma once
#include "skl/graphics/base.hpp"
#include "skl/memory/memory.hpp"

#define NAMESPACE_OPENGL_BEGIN \
    NAMESPACE_GRAPHICS_BEGIN   \
    namespace opengl {

#define NAMESPACE_OPENGL_END \
    NAMESPACE_GRAPHICS_END   \
    }


NAMESPACE_OPENGL_BEGIN
class IOpenGLAPI : public IGraphicsAPI {
public:
    IOpenGLAPI() = default;
    ~IOpenGLAPI() override = default;

    SKL_DEFAULT_COPYMOVE(IOpenGLAPI);
    [[nodiscard]] Backend backend() const noexcept override { return Backend::opengl; }
};
NAMESPACE_OPENGL_END
