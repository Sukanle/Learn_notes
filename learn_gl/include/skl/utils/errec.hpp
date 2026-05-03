#pragma once
#include <stdint.h>
#include "skl/utils/base.hpp"
#include "skl/error_code.h"

NAMESPACE_UTILS_BEGIN
enum class errc : uint16_t {

#define SKL_UTILS_ERROR_CODES(X)                                                                                      \
    /* === Common / Standard Error (0x0001-0x00FF) === */                                                             \
    X(invalid_argument, "Invalid argument", 0x0016, std::errc::invalid_argument)                                      \
    X(unknown, "Unknown utils error.", 0x00FF, SKL_STDERR_FALLBACK)                                                   \
                                                                                                                      \
    /* === Language package error (0x0101-0x01FF) === */                                                              \
    X(lang_no_such_file, "The language package file does not exist.", 0x0101, std::errc::no_such_file_or_directory)   \
                                                                                                                      \
    /* === Dynamic library error (0x0301-0x03FF) === */                                                               \
    X(dl_open_failed, "Failed to open dynamic library.", 0x0301, std::errc::no_such_file_or_directory)                \
    X(dl_symbol_not_found, "Failed to resolve symbol from dynamic library.", 0x0302,                                  \
      std::errc::function_not_supported)                                                                              \
    X(dl_api_load_failed, "Failed to load dynamic library API.", 0x0303, std::errc::function_not_supported)           \
                                                                                                                      \
    /* === Simd-JSON error (0x0201-0x02FF) === */                                                                     \
    X(json_capacity, "This parser can't support a document that big.", 0x0201, std::errc::value_too_large)            \
    X(json_memory_allocation, "Error allocating memory, most likely out of memory.", 0x0202,                          \
      std::errc::not_enough_memory)                                                                                   \
    X(json_tape_error, "Something went wrong, this is a generic error. Fatal/unrecoverable error.", 0x0203,           \
      SKL_STDERR_FALLBACK)                                                                                            \
    X(json_depth_error, "Your document exceeds the user-specified depth limitation.", 0x0204,                         \
      std::errc::value_too_large)                                                                                     \
    X(json_string_error, "Problem while parsing a string.", 0x0205, std::errc::invalid_argument)                      \
    X(json_t_atom_error, "Problem while parsing an atom starting with the letter 't', expected: true", 0x0206,        \
      std::errc::invalid_argument)                                                                                    \
    X(json_f_atom_error, "Problem while parsing an atom starting with the letter 'f', expected: false", 0x0207,       \
      std::errc::invalid_argument)                                                                                    \
    X(json_n_atom_error, "Problem while parsing an atom starting with the letter 'n', expected: null", 0x0208,        \
      std::errc::invalid_argument)                                                                                    \
    X(json_number_error, "Problem while parsing a number.", 0x0209, std::errc::invalid_argument)                      \
    X(json_bigint_error, "The integer value exceeds 64 bits.", 0x020A, std::errc::result_out_of_range)                \
    X(json_utf8_error, "The input is not valid UTF-8.", 0x020B, std::errc::illegal_byte_sequence)                     \
    X(json_uninitialized, "Unknown error, or uninitialized document.", 0x020C, SKL_STDERR_FALLBACK)                   \
    X(json_empty, "No structural element found.", 0x020D, std::errc::invalid_argument)                                \
    X(json_unescaped_chars, "Found unescaped characters in a string.", 0x020E, std::errc::invalid_argument)           \
    X(json_unclosed_string, "Missing quote at the end.", 0x020F, std::errc::invalid_argument)                         \
    X(json_unsupported_architecture, "Unsupported architecture.", 0x0210, std::errc::not_supported)                   \
    X(json_incorrect_type, "JSON element has a different type than user expected.", 0x0211,                           \
      std::errc::invalid_argument)                                                                                    \
    X(json_number_out_of_range, "JSON number does not fit in 64 bits.", 0x0212, std::errc::result_out_of_range)       \
    X(json_index_out_of_bounds, "JSON array index too large.", 0x0213, std::errc::invalid_argument)                   \
    X(json_no_such_field, "JSON field not found in object.", 0x0214, std::errc::no_such_file_or_directory)            \
    X(json_io_error, "Error reading a file.", 0x0215, std::errc::io_error)                                            \
    X(json_invalid_json_pointer, "Invalid JSON pointer syntax.", 0x0216, std::errc::invalid_argument)                 \
    X(json_invalid_uri_fragment, "Invalid URI fragment.", 0x0217, std::errc::invalid_argument)                        \
    X(json_unexpected_error, "Indicative of a bug in simdjson.", 0x0218, SKL_STDERR_FALLBACK)                         \
    X(json_parser_in_use, "Parser is already in use.", 0x0219, std::errc::device_or_resource_busy)                    \
    X(json_out_of_order_iteration, "Tried to iterate an array or object out of order.", 0x021A,                       \
      std::errc::invalid_argument)                                                                                    \
    X(json_insufficient_padding, "The JSON doesn't have enough padding for simdjson to safely parse it.", 0x021B,     \
      std::errc::invalid_argument)                                                                                    \
    X(json_incomplete_array_or_object, "The document ends early. Fatal/unrecoverable error.", 0x021C,                 \
      std::errc::invalid_argument)                                                                                    \
    X(json_scalar_document_as_value, "A scalar document is treated as a value.", 0x021D, std::errc::invalid_argument) \
    X(json_out_of_bounds, "Attempted to access location outside of document.", 0x021E, std::errc::invalid_argument)   \
    X(json_trailing_content, "Unexpected trailing content in the JSON input.", 0x021F, std::errc::invalid_argument)   \
    X(json_out_of_capacity, "The capacity was exceeded, we cannot allocate enough memory.", 0x0220,                   \
      std::errc::not_enough_memory)                                                                                   \
    X(json_num_error_codes, "Internal Sentinel (NUM_ERROR_CODES).", 0x0221, SKL_STDERR_FALLBACK)

    SKL_UTILS_ERROR_CODES(SKL_ERR_X_ENUM)
};
NAMESPACE_UTILS_END
