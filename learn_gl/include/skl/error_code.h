#pragma once
#include <system_error>
namespace skl {
class error_category : public std::error_category {
public:
    virtual ~error_category() = default;
    const char *name() const noexcept override = 0;
    std::string message(int ev) const override = 0;
    virtual const char *moduleId() const noexcept = 0;
    virtual const char *moduleName() const noexcept = 0;
    std::error_condition default_error_condition(int ev) const noexcept override = 0;

protected:
    error_category() = default;
};
}   // namespace skl
