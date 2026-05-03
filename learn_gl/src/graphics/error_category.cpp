#include "skl/graphics/error_category.hpp"
#include "skl/utils/lang.hpp"
#define SKL_ERR_ENUM errc

NAMESPACE_GRAPHICS_BEGIN
const char *error_category::name() const noexcept {
    return "skl.graphics";
}
const char *error_category::moduleId() const noexcept {
    return "graphics";
}
const char *error_category::moduleName() const noexcept {
    return "Graphics Subsystem";
}
std::string error_category::message(int ev) const {
    std::error_code i18n_ec;
#ifndef SKL_MODULE_LANG
    auto localized = utils::global_i18n_loader().find(i18n_ec, moduleId(), (uint32_t)ev);
    if (!localized.empty()) return std::string(localized);
#endif

    switch ((errc)ev) {
        SKL_GRAPHICS_ERROR_CODES(SKL_ERR_X_MSG)
        default: return "Invalid error, illegal out-of-bounds";
    }
}


std::error_condition error_category::default_error_condition(int ev) const noexcept {
    if (!ev) return {0, std::generic_category()};
    switch ((errc)ev) {
        SKL_GRAPHICS_ERROR_CODES(SKL_ERR_X_MAP)
        default: return {ev, *this};
    }
}
NAMESPACE_GRAPHICS_END

NAMESPACE_SKL_BEGIN
std::error_code make_error_code(graphics::errc ec) {
    return {(int)ec, graphics::error_category::instance()};
}
NAMESPACE_SKL_END
