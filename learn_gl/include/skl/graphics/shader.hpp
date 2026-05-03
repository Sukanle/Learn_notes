#include <filesystem>

#include "skl/graphics/base.hpp"
#include "skl/utils/hash.hpp"

NAMESPACE_GRAPHICS_BEGIN

using UniformID = uint64_t;

class IShader : virtual public IAttribute {
public:
    IShader() = default;
    ~IShader() override = default;

    SKL_DEFAULT_COPYMOVE(IShader);

    IShader &build(std::error_code &ec, const Char *vertexPath, const Char *fragmentPath) noexcept;
    IShader &build(std::error_code &ec, const std::filesystem::path &vertexPath,
                   const std::filesystem::path &fragmentPath) noexcept;
    virtual void use() const noexcept = 0;
private:
};
inline UniformID makeUniformID(String_view name) noexcept {
    static utils::Hash64 hash;
    ;
    return hash(name);
}

NAMESPACE_GRAPHICS_END
