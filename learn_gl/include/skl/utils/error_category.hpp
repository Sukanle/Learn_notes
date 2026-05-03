#pragma once

#include "skl/utils/errec.hpp"

NAMESPACE_UTILS_BEGIN
class error_category : public skl::error_category {
public:
    static const error_category &instance() {
        static error_category inst;
        return inst;
    }

    [[nodiscard]] const char *name() const noexcept override;
    [[nodiscard]] const char *moduleId() const noexcept override;
    [[nodiscard]] const char *moduleName() const noexcept override;

    [[nodiscard]] std::string message(int ev) const override;

    [[nodiscard]] std::error_condition default_error_condition(int ev) const noexcept override;

private:
    error_category() = default;
};
NAMESPACE_UTILS_END

NAMESPACE_SKL_BEGIN
std::error_code make_error_code(utils::errc ec);
NAMESPACE_SKL_END
