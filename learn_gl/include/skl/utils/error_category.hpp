#pragma once
#include "skl/utils/errec.hpp"
#include <stdint.h>

namespace skl {
class utils_err : public error_category {
public:
    static const utils_err &instance() {
        static utils_err inst;
        return inst;
    }

    [[nodiscard]] const char *name() const noexcept override;
    [[nodiscard]] const char *moduleId() const noexcept override;
    [[nodiscard]] const char *moduleName() const noexcept override;

    [[nodiscard]] std::string message(int ev) const override;

    [[nodiscard]] std::error_condition default_error_condition(int ev) const noexcept override;

private:
    utils_err() = default;
};

std::error_code make_error_code(utils_ec ec);

}   // namespace skl
