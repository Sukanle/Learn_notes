#pragma once
#include "skl/graphics/errec.hpp"

namespace skl {
class graphics_err : public error_category {
public:
    static const graphics_err &instance() {
        static graphics_err inst;
        return inst;
    }

    [[nodiscard]] const char *name() const noexcept override;
    [[nodiscard]] const char *moduleId() const noexcept override;
    [[nodiscard]] const char *moduleName() const noexcept override;

    [[nodiscard]] std::string message(int ev) const override;

    [[nodiscard]] std::error_condition default_error_condition(int ev) const noexcept override;

private:
    graphics_err() = default;
};

std::error_code make_error_code(graphics_ec ec);

}   // namespace skl
