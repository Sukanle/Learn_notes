#ifdef SKL_LANG_DEMO_DYNAMIC
#ifndef SKL_MODULE_LANG
#define SKL_MODULE_LANG
#endif
#include "skl/utils/dl.hpp"
#include "skl/utils/lang.hpp"
#else
#include "skl/graphics/error_category.hpp"
#include "skl/utils/error_category.hpp"
#endif

#include <stdio.h>
#include <stdlib.h>

#include <filesystem>
#include <string>

#if defined(DEBUG) || defined (__DEBUG__) || defined(_DEBUG)
#define LIB_SUFFIX "debug"
#else
#define LIB_SUFFIX "release"
#endif

#if defined (_WIN32)
#define LIB_EXTENSION "dll"
#define LIB_PLATFORM "windows"
#elif defined (__APPLE__)
#define LIB_EXTENSION "dylib"
#define LIB_PLATFORM "macosx"
#else
#define LIB_EXTENSION "so"
#define LIB_PLATFORM "linux"
#endif

// check CPU architecture (arm64/x86_64/i386) for correct dynamic library loading
#if defined(__aarch64__) || defined(_M_ARM64)
#define LIB_ARCH "arm64"
#elif defined(__i386__) || defined(_M_IX86)
#define LIB_ARCH "x86"
#elif defined(__x86_64__) || defined(_M_X64)
#define LIB_ARCH "x86_64"
#else
error "Unsupported architecture"
#endif


namespace {

void set_runtime_lang(const std::string &lang) {
#if defined(_WIN32)
    _putenv_s("SKL_LANG", lang.c_str());
#else
    setenv("SKL_LANG", lang.c_str(), 1);
#endif
}

void print_case(const char *title, const std::error_code &ec) {
    printf("%s\n", title);
    printf("  category: %s\n", ec.category().name());
    printf("  value   : 0x%X\n", ec.value());
    printf("  message : %s\n\n", ec.message().c_str());
}

void print_usage(const char *argv0) {
#ifdef SKL_LANG_DEMO_DYNAMIC
    printf("Usage: %s [lang] [lang_library_path]\n", argv0);
    printf("Example: %s zh-CN ./build/%s/%s/%s/libskl-lang.%s\n\n", argv0, LIB_PLATFORM, LIB_ARCH ,LIB_SUFFIX, LIB_EXTENSION);
#else
    printf("Usage: %s [lang]\n", argv0);
    printf("Example: %s zh-CN\n", argv0);
    printf("Example: %s en-US\n\n", argv0);
#endif
}

} // namespace

int main(int argc, char **argv) {
    std::string lang = "zh-CN";
    if (argc >= 2) {
        lang = argv[1];
    }

    if (lang == "-h" || lang == "--help") {
        print_usage(argv[0]);
        return 0;
    }

    set_runtime_lang(lang);

#ifdef SKL_LANG_DEMO_DYNAMIC
    std::filesystem::path lib_path;
    if (argc >= 3) {
        // lib_path = argv[2];
        lib_path = std::filesystem::path(argv[2]);
    } else {
#if defined(_WIN32)
        // lib_path = "./build/windows/x64/debug/skl-lang.dll";
        lib_path = std::filesystem::current_path() / "build" / LIB_PLATFORM / LIB_ARCH / LIB_SUFFIX / ("libskl-lang." + std::string(LIB_EXTENSION));
#elif defined(__APPLE__)
        // lib_path = "./build/macosx/arm64/debug/libskl-lang.dylib";
        lib_path = std::filesystem::current_path() / "build" / LIB_PLATFORM / LIB_ARCH / LIB_SUFFIX / ("libskl-lang." + std::string(LIB_EXTENSION));
#else
        // lib_path = "./build/linux/x86_64/debug/libskl-lang.so";
        lib_path = std::filesystem::current_path() / "build" / LIB_PLATFORM / LIB_ARCH / LIB_SUFFIX / ("libskl-lang." + std::string(LIB_EXTENSION));
#endif
    }

    printf("[I18N Demo - Dynamic Load] runtime language = %s\n", lang.c_str());
    printf("[I18N Demo - Dynamic Load] lang library path = %s\n", lib_path.c_str());

    std::error_code ec;
    skl::DynLib lib;
    lib.open(lib_path.c_str(), ec);
    if (ec) {
        printf("open dynamic library failed: %s\n", ec.message().c_str());
        return 1;
    }

    SKL_I18N_C_API api{};
    if (!SKL_I18N_loader(lib, api, ec)) {
        printf("load i18n api failed: %s\n", ec.message().c_str());
        return 2;
    }

    const char *message = nullptr;
    size_t message_size = 0;
    const int rc = api.find_ex("utils", 0x0016U, &message, &message_size);
    printf("find_ex(submodule=utils, code=0x0016) rc=%d\n", rc);
    if (rc == 0 && message) {
        // printf("message: %.*s\n", static_cast<int>(message_size), message);
    } else if (api.last_error) {
        printf("last_error: %s\n", api.last_error());
    }

    return 0;
#else
    printf("[I18N Demo] runtime language = %s\n", lang.c_str());
    puts("Expected behavior:");
    puts("- Existing language pack/submodule -> localized message");
    puts("- Missing language pack/submodule  -> fallback hardcoded English\n");

    // utils errors
    print_case("[utils] invalid_argument", skl::make_error_code(skl::utils_ec::invalid_argument));
    print_case("[utils] json_utf8_error", skl::make_error_code(skl::utils_ec::json_utf8_error));
    print_case("[utils] unknown", skl::make_error_code(skl::utils_ec::unknown));

    // graphics errors
    print_case("[graphics] shader_compile_failed", skl::make_error_code(skl::graphics_ec::shader_compile_failed));
    print_case("[graphics] texture_unit_mismatch", skl::make_error_code(skl::graphics_ec::texture_unit_mismatch));
    print_case("[graphics] unknown", skl::make_error_code(skl::graphics_ec::unknown));

    // unknown code should always use fallback path
    std::error_code invalid_graphics{0x0BAD, skl::graphics_err::instance()};
    print_case("[graphics] invalid_error 0x0BAD", invalid_graphics);
    std::error_code invalid_utils{0xFBAD, skl::utils_err::instance()};
    print_case("[utils] invalid_error 0xFBAD", invalid_utils);

    return 0;
#endif
}
