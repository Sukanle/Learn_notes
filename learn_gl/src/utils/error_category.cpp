#include "skl/utils/error_category.hpp"
#ifdef SKL_MODULE_LANG
#include "skl/utils/dl.hpp"
#endif
#include "skl/utils/lang.hpp"

#define SKL_ERR_ENUM utils_ec

namespace skl {

const char *utils_err::name() const noexcept {
    return "skl.utils";
}
const char *utils_err::moduleId() const noexcept {
    return "utils";
}
const char *utils_err::moduleName() const noexcept {
    return "Utils Subsystem";
}
std::string utils_err::message(int ev) const {
    std::error_code i18n_ec;
#ifndef SKL_MODULE_LANG
    auto localized = global_i18n_loader().find(i18n_ec, moduleId(), static_cast<uint32_t>(ev));
#else
    auto 
#endif
    if (!localized.empty()) return std::string(localized);

    // 使用静态映射表，避免大量 switch
    switch ((utils_ec)ev) {
        SKL_UTILS_ERROR_CODES(SKL_ERR_X_MSG)
        default: return "Invalid error, illegal out-of-bounds";
    }
}

std::error_condition utils_err::default_error_condition(int ev) const noexcept {
    if (!ev) return {0, std::generic_category()};
    // 按模块映射到最接近的 std::errc
    switch ((utils_ec)ev) {
        SKL_UTILS_ERROR_CODES(SKL_ERR_X_MAP)
        default: return {ev, *this};
    }
}

std::error_code make_error_code(utils_ec ec) {
    return {(int)ec, utils_err::instance()};
}
}   // namespace skl
