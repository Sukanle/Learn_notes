#include "skl/utils/dl.hpp"
#include "skl/utils/error_category.hpp"

#if defined(_WIN32)
#include <libloaderapi.h>
#define DLOpen(path, mode) LoadLibrary(path)
#define DLClose(hd) FreeLibrary((HMODULE)(hd))
#else
#include <dlfcn.h>
#define DLOpen(path, mode) dlopen(path, mode)
#define DLClose(hd) dlclose(hd)
#endif

namespace {
inline void DLerror() noexcept{
#if defined(_WIN32)
    const DWORD err_code = GetLastError();
    const char *err_msg = nullptr;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, err_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char *)&err_msg, 0, nullptr);
    (void)fprintf(stderr, "Dynamic library error code %lu: %s\n", err_code, err_msg ? err_msg : "unknown error");
    if (err_msg) LocalFree((HLOCAL)err_msg);
#else
    perror("Dynamic library error(C standard):");
    fprintf(stderr, "Dynamic library error(POSIX): %s\n", dlerror());
#endif
}
}

NAMESPACE_UTILS_BEGIN
SKLErr DynLib::open(const skl::Char *path, dyn_t **out_handle) noexcept {
    if (!path || !out_handle) { return (SKLErr)errc::invalid_argument; }
    if (out_handle) *out_handle = nullptr;

    auto *h = DLOpen(path, RTLD_NOW | RTLD_LOCAL);

    if (!h) {
#if defined(DEBUG) || defined (__DEBUG__) || defined(_DEBUG)
        DLerror();
#endif
        return (SKLErr)errc::dl_open_failed; 
    }
    *out_handle = h;
    return EXIT_SUCCESS;
}

SKLErr DynLib::symbol(dyn_t *handle, const skl::Char *symbol, dynfn_t *out_symbol) noexcept {
    if (!handle || !symbol || !out_symbol) { return (SKLErr)errc::invalid_argument; }
    *out_symbol = nullptr;

#if defined(_WIN32)
    FARPROC p = GetProcAddress((HMODULE)handle, symbol);
#else
    dlerror();
    dyn_t *p = dlsym(handle, symbol);
#endif
    if (!p) {
#if defined(DEBUG) || defined (__DEBUG__) || defined(_DEBUG)
        DLerror();
#endif
        return (SKLErr)errc::dl_symbol_not_found; 
    }
    *out_symbol = (dynfn_t)p;
    return EXIT_SUCCESS;
}

void DynLib::close(dyn_t *handle) noexcept {
    if (!handle) return;
    DLClose(handle);
}

void DynLib::open(skl::String_view path, std::error_code &ec) noexcept {
    close();
    ec.clear();

    if (path.empty()) {
        ec = make_error_code(errc::invalid_argument);
        return;
    }

    const skl::String path_str(path);
    dyn_t *h = nullptr;
    const SKLErr rc = open(path_str.c_str(), &h);
    if (rc != EXIT_SUCCESS) {
        ec = make_error_code((errc)rc);
        return;
    }

    _handle = h;
}

void DynLib::close() noexcept {
    if (!_handle) return;
    close(_handle);
    _handle = nullptr;
}

DynLib::dynfn_t DynLib::symbol(const skl::Char *name, std::error_code &ec) const noexcept {
    ec.clear();

    if (!_handle || !name) {
        ec = make_error_code(errc::invalid_argument);
        return nullptr;
    }

    dynfn_t pfn = nullptr;
    const SKLErr rc = symbol(_handle, name, &pfn);
    if (rc != EXIT_SUCCESS) {
        ec = make_error_code((errc)rc);
        return nullptr;
    }
    return pfn;
}

NAMESPACE_UTILS_END
