#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#ifndef SKL_MODULE_LANG
#  include <memory>
#  include <string>
#  include <string_view>
#  include <system_error>
#  include <unordered_map>

#  include "skl/utils/base.hpp"
NAMESPACE_UTILS_BEGIN
class I18NLoader {
private:
    struct package;
    std::unique_ptr<package> impl;
    bool loaded = false;
    // Own message storage to avoid dangling views from parser-backed buffers.
    std::unordered_map<std::string, std::unordered_map<uint32_t, std::string>> index_map;

public:
    explicit I18NLoader(std::error_code &ec, const std::string &file_path, std::string_view lang);
    ~I18NLoader();
    I18NLoader(const I18NLoader &) = delete;
    I18NLoader &operator=(const I18NLoader &) = delete;
    I18NLoader(I18NLoader &&) noexcept;
    I18NLoader &operator=(I18NLoader &&) noexcept;

    // 查找接口
    [[nodiscard]] std::string_view find(std::error_code &ec, std::string_view submodule, uint32_t code) const noexcept;

    [[nodiscard]] bool is_loaded() const noexcept { return loaded; }
};

// 获取全局本地化加载器（懒加载，失败时保持未加载状态）
[[nodiscard]] const I18NLoader &global_i18n_loader() noexcept;
NAMESPACE_UTILS_END
#else
#  include "skl/utils/dl.hpp"
#  include "skl/utils/error_category.hpp"
#  ifdef __cplusplus
typedef int (*SKL_I18N_find_ex_fn)(const char *, uint32_t, const char **, size_t *);
typedef const char *(*SKL_I18N_last_error_fn)(void);

typedef struct SKL_I18N_C_API {
    SKL_I18N_find_ex_fn find_ex;
    SKL_I18N_last_error_fn last_error;
} SKL_I18N_C_API;

#    define SKL_I18N_SYMBOL_FIND_EX "skl_i18n_find_ex"
#    define SKL_I18N_SYMBOL_LAST_ERROR "skl_i18n_last_error_message"

typedef bool (*SKL_I18N_loader_fn)(const skl::utils::DynLib &, SKL_I18N_C_API &, std::error_code &);

static inline bool SKL_I18N_loader(const skl::utils::DynLib &lib, SKL_I18N_C_API &out_api, std::error_code &ec) {
    ec.clear();
    if (!lib.is_open()) {
        ec = skl::make_error_code(skl::utils::errc::invalid_argument);
        return false;
    }

    out_api.find_ex = lib.symbol_as<SKL_I18N_find_ex_fn>(SKL_I18N_SYMBOL_FIND_EX, ec);
    if (ec || !out_api.find_ex) {
        if (!ec) ec = skl::make_error_code(skl::utils::errc::dl_symbol_not_found);
        return false;
    }

    out_api.last_error = lib.symbol_as<SKL_I18N_last_error_fn>(SKL_I18N_SYMBOL_LAST_ERROR, ec);
    if (ec || !out_api.last_error) {
        if (!ec) ec = skl::make_error_code(skl::utils::errc::dl_symbol_not_found);
        out_api.find_ex = nullptr;
        return false;
    }

    return true;
}
#  endif
#endif   // !SKL_MODULE_LANG
