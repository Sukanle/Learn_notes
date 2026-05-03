#include "skl/graphics/base.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image.h>


NAMESPACE_GRAPHICS_BEGIN

class ITexture2D : virtual public IAttribute {
public:
    ITexture2D() = default;
    ~ITexture2D() override = default;

    SKL_DEFAULT_MOVEABLE(ITexture2D);
    SKL_DELETE_COPYABLE(ITexture2D);
 
    static void setFlipY(bool flip = true) noexcept { stbi_set_flip_vertically_on_load(flip); }

    virtual ITexture2D &bind() noexcept = 0;
};

NAMESPACE_GRAPHICS_END
