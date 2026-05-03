#pragma once
#ifdef SKL_MACRO_UNUSED
#  define SKL_UNUSED_VAR(var) ((void)var)
#  define SKL_UNUSED_RET(fn) ((void)fn)
#endif
#ifdef __cplusplus
#  ifndef SKL_CURRENT_VERSION
#    define SKL_CURRENT_VERSION 0X00000001
#    define SKL_NAMESPACE_CURVER v0
#  endif

#  ifndef NAMESPACE_SKL_BEGIN
#    define NAMESPACE_SKL_BEGIN \
        namespace skl {         \
        inline namespace SKL_NAMESPACE_CURVER {

#    define NAMESPACE_SKL_END \
        }                     \
        }
#  endif

#  ifndef SKL_DELETE_COPYABLE
#    define SKL_DELETE_COPYABLE(CLASS)          \
        CLASS(const CLASS &) noexcept = delete; \
        CLASS &operator=(const CLASS &) noexcept = delete;
#  endif

#  ifndef SKL_DELETE_MOVEABLE
#    define SKL_DELETE_MOVEABLE(CLASS)     \
        CLASS(CLASS &&) noexcept = delete; \
        CLASS &operator=(CLASS &&) noexcept = delete;
#  endif

#  ifndef SKL_DEFAULT_COPYABLE
#    define SKL_DEFAULT_COPYABLE(CLASS)          \
        CLASS(const CLASS &) noexcept = default; \
        CLASS &operator=(const CLASS &) noexcept = default;
#  endif

#  ifndef SKL_DEFAULT_MOVEABLE
#    define SKL_DEFAULT_MOVEABLE(CLASS)     \
        CLASS(CLASS &&) noexcept = default; \
        CLASS &operator=(CLASS &&) noexcept = default;
#  endif

#  ifndef SKL_DEFAULT_COPYMOVE
#    define SKL_DEFAULT_COPYMOVE(CLASS) \
        SKL_DEFAULT_COPYABLE(CLASS)     \
        SKL_DEFAULT_MOVEABLE(CLASS)
#  endif

#  ifndef SKL_DELETE_COPYMOVE
#    define SKL_DELETE_COPYMOVE(CLASS) \
        SKL_DELETE_COPYABLE(CLASS)     \
        SKL_DELETE_MOVEABLE(CLASS)
#  endif

#  include <version>

#  if defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG__)
#    define SKL_DEBUG
#    import <fmt/format.h>
#    define SKL_PRINT(...) fmt::print(__VA_ARGS__)
#    define SKL_FORMAT(...) fmt::format(__VA_ARGS__)
#  else
#    define SKL_PRINT(...)
#    define SKL_FORMAT(...)
#  endif

#  ifdef __cpp_lib_function_ref
#    define SKL_FUNCTION_COPEABLE std::copyable_function
#    define SKL_FUNCTION_MOVEABLE fu2::move_only_function
#    define SKL_FUNCTION_VIEW fu2::function_ref
#  else
#    import <function2/function2.hpp>
#    define SKL_FUNCTION_COPEABLE fu2::function
#    define SKL_FUNCTION_MOVEABLE fu2::unique_function
#    define SKL_FUNCTION_VIEW fu2::function_view
#  endif
#  define SKL_C_API extern "C"
#else
#  define SKL_C_API
#endif
