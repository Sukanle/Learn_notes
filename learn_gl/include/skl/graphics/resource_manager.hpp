#pragma once

#include "skl/memory/memory.hpp"
#include "skl/graphics/gl/texture.hpp"

NAMESPACE_GRAPHICS_BEGIN

using TextureHandle = memory::Handle<opengl::Texture2D>;
using TextureManager = memory::ResourceManager<opengl::Texture2D>;



TextureManager &TextureMgr() noexcept;
NAMESPACE_GRAPHICS_END
