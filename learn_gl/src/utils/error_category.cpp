#include "skl/utils/error_category.hpp"
#ifdef SKL_MODULE_LANG
#include "skl/utils/dl.hpp"
#endif
#include "skl/utils/lang.hpp"

#define SKL_ERR_ENUM errc

NAMESPACE_UTILS_BEGIN
const char *error_category::name() const noexcept {
    return "skl.utils";
}
const char *error_category::moduleId() const noexcept {
    return "utils";
}
const char *error_category::moduleName() const noexcept {
    return "Utils Subsystem";
}
std::string error_category::message(int ev) const {
    std::error_code i18n_ec;
#ifndef SKL_MODULE_LANG
    auto localized = global_i18n_loader().find(i18n_ec, moduleId(), static_cast<uint32_t>(ev));
    if (!localized.empty() && i18n_ec) return std::string(localized);
#endif

    switch ((errc)ev) {
        SKL_UTILS_ERROR_CODES(SKL_ERR_X_MSG)
        default: return "Invalid error, illegal out-of-bounds";
    }
}

std::error_condition error_category::default_error_condition(int ev) const noexcept {
    if (!ev) return {0, std::generic_category()};
    // 按模块映射到最接近的 std::errc
    switch ((errc)ev) {
        SKL_UTILS_ERROR_CODES(SKL_ERR_X_MAP)
        default: return {ev, *this};
    }
}
NAMESPACE_UTILS_END

NAMESPACE_SKL_BEGIN
std::error_code make_error_code(utils::errc ec) {
    return {(int)ec, utils::error_category::instance()};
}
NAMESPACE_SKL_END
