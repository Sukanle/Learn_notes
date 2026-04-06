#pragma once
#include <system_error>

#define SKL_STDERR_FALLBACK {ev, *this}
#define SKL_ERR_X_ENUM(name, msg, val, map) name = (val),
#define SKL_ERR_X_MSG(name, msg, val, map) \
    case SKL_ERR_ENUM::name: return msg;
#define SKL_ERR_X_MAP(name, msg, val, map) \
    case SKL_ERR_ENUM::name: return map;

namespace skl {
class error_category : public std::error_category {
public:
    virtual ~error_category() = default;
    [[nodiscard]] const char *name() const noexcept override = 0;
    [[nodiscard]] std::string message(int ev) const override = 0;
    [[nodiscard]] virtual const char *moduleId() const noexcept = 0;
    [[nodiscard]] virtual const char *moduleName() const noexcept = 0;
    [[nodiscard]] std::error_condition default_error_condition(int ev) const noexcept override = 0;

protected:
    error_category() = default;
};
}   // namespace skl
