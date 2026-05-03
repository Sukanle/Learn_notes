#include "skl/graphics/resource_manager.hpp"

NAMESPACE_GRAPHICS_BEGIN
TextureManager &TextureMgr() noexcept {
    static TextureManager mgr;
    return mgr;
}
NAMESPACE_GRAPHICS_END
