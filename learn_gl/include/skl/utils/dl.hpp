#pragma once
#include <stdlib.h>
#include <system_error>

#include "skl/base.hpp"

namespace skl {
class DynLib {
public:
    using dyn_t = void;
    using dynfn_t = void(*)();
    DynLib() = default;
    explicit DynLib(skl::String_view path, std::error_code &ec) { open(path, ec); }

    DynLib(const DynLib &) = delete;
    DynLib &operator=(const DynLib &) = delete;

    DynLib(DynLib &&other) noexcept
        : _handle(other._handle) {
        other._handle = nullptr;
    }

    DynLib &operator=(DynLib &&other) noexcept {
        if (this == &other) return *this;
        close();
        _handle = other._handle;
        other._handle = nullptr;
        return *this;
    }

    ~DynLib() { close(); }

    [[nodiscard]] bool is_open() const noexcept { return _handle != nullptr; }
    [[nodiscard]] dyn_t *native_handle() const noexcept { return _handle; }

    void open(skl::String_view path, std::error_code &ec) noexcept;
    void close() noexcept;
    dynfn_t symbol(const skl::Char *name, std::error_code &ec) const noexcept;
    template<typename Fn>
    Fn symbol_as(const skl::Char *name, std::error_code &ec) const {
        return reinterpret_cast<Fn>(symbol(name, ec));
    }

private:
    dyn_t *_handle = nullptr;
    static SKLErr open(const skl::Char *path, dyn_t **out_handle) noexcept;
    static SKLErr symbol(dyn_t *handle, const skl::Char *symbol, dynfn_t *out_symbol) noexcept;
    static void close(dyn_t *handle) noexcept;
};

}   // namespace skl
