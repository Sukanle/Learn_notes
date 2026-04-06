#include "skl/graphics/error_category.hpp"
#include "skl/utils/lang.hpp"
#define SKL_ERR_ENUM graphics_ec

namespace skl {

const char *graphics_err::name() const noexcept {
    return "skl.graphics";
}
const char *graphics_err::moduleId() const noexcept {
    return "graphics";
}
const char *graphics_err::moduleName() const noexcept {
    return "Graphics Subsystem";
}
std::string graphics_err::message(int ev) const {
    std::error_code i18n_ec;
    auto localized = global_i18n_loader().find(i18n_ec, moduleId(), (uint32_t)ev);
    if (!localized.empty()) return std::string(localized);

    switch ((graphics_ec)ev) {
        SKL_GRAPHICS_ERROR_CODES(SKL_ERR_X_MSG)
        default: return "Invalid error, illegal out-of-bounds";
    }
}


std::error_condition graphics_err::default_error_condition(int ev) const noexcept {
    if (!ev) return {0, std::generic_category()};
    switch ((graphics_ec)ev) {
        SKL_GRAPHICS_ERROR_CODES(SKL_ERR_X_MAP)
        default: return {ev, *this};
    }
}

std::error_code make_error_code(graphics_ec ec) {
    return {(int)ec, graphics_err::instance()};
}
}   // namespace skl
