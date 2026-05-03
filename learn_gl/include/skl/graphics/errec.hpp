#pragma once
#import "skl/error_code.h"
#import "skl/graphics/base.hpp"

NAMESPACE_GRAPHICS_BEGIN
enum class errc : uint16_t {
#define SKL_GRAPHICS_ERROR_CODES(X)                                                                                \
    /* === Common (0x0001-0x00FF) === */                                                                           \
    X(invalid_argument, "Graphics Invalid argument", 0x0016, std::errc::invalid_argument)                          \
    X(unknown, "Unknown graphics error", 0x00FF, SKL_STDERR_FALLBACK)                                              \
                                                                                                                   \
    /* === Device (0x0101-0x01FF) === */                                                                           \
    X(device_lost, "GPU device lost", 0x0101, std::errc::no_such_device)                                           \
    X(device_timeout, "GPU operation timed out", 0x0102, SKL_STDERR_FALLBACK)                                      \
    X(out_of_memory, "Out of GPU memory", 0x0103, std::errc::not_enough_memory)                                    \
    X(device_initialization_failed, "Device initialization failed", 0x0104, SKL_STDERR_FALLBACK)                   \
    X(feature_unsupported, "Required GPU feature not supported", 0x0105, std::errc::not_supported)                 \
    X(context_lost, "Rendering context lost", 0x0106, SKL_STDERR_FALLBACK)                                         \
                                                                                                                   \
    /* === Shader (0x0201-0x02FF) === */                                                                           \
    X(shader_compile_failed, "Shader compilation failed", 0x0201, std::errc::invalid_argument)                     \
    X(shader_link_failed, "Shader linking failed", 0x0202, std::errc::invalid_argument)                            \
    X(shader_validation_failed, "Shader validation failed", 0x0203, std::errc::invalid_argument)                   \
    X(shader_entry_point_not_found, "Shader entry point not found", 0x0204, std::errc::invalid_argument)           \
    X(shader_source_load_failed, "Failed to load shader source file", 0x0205, std::errc::io_error)                 \
    X(shader_invalid_format, "Invalid shader binary format", 0x0206, SKL_STDERR_FALLBACK)                          \
    X(shader_resource_limit_exceeded, "Shader resource limit exceeded", 0x0207, std::errc::not_enough_memory)      \
                                                                                                                   \
    /* === Texture (0x0301-0x03FF) === */                                                                          \
    X(texture_creation_failed, "Texture creation failed", 0x0301, std::errc::invalid_argument)                     \
    X(texture_format_unsupported, "Texture format not supported", 0x0302, std::errc::invalid_argument)             \
    X(texture_upload_failed, "Texture data upload failed", 0x0303, std::errc::io_error)                            \
    X(texture_mipmap_generation_failed, "Mipmap generation failed", 0x0304, SKL_STDERR_FALLBACK)                   \
    X(texture_boundary_invalid, "Invalid texture boundary mode", 0x0305, std::errc::invalid_argument)              \
    X(texture_size_exceeded, "Texture size exceeds hardware limit", 0x0306, std::errc::value_too_large)            \
    X(texture_array_layer_exceeded, "Texture array layers exceeded", 0x0307, SKL_STDERR_FALLBACK)                  \
    X(texture_unit_exhausted, "No available texture image units", 0x0308, std::errc::no_buffer_space)              \
    X(texture_unit_not_bound, "Texture unit not bound to valid texture", 0x0309, std::errc::invalid_argument)      \
    X(texture_unit_mismatch, "Texture unit bound to unexpected texture ID", 0x030A, std::errc::invalid_argument)   \
                                                                                                                   \
    /* === Buffer (0x0401-0x04FF) === */                                                                           \
    X(buffer_creation_failed, "Buffer allocation failed", 0x0401, std::errc::invalid_argument)                     \
    X(buffer_mapping_failed, "Buffer memory mapping failed", 0x0402, std::errc::io_error)                          \
    X(buffer_binding_failed, "Buffer binding failed", 0x0403, std::errc::invalid_argument)                         \
    X(buffer_overflow, "Buffer write overflow", 0x0404, std::errc::invalid_argument)                               \
    X(buffer_range_invalid, "Invalid buffer access range", 0x0405, std::errc::invalid_argument)                    \
                                                                                                                   \
    /* === Pipeline (0x0501-0x05FF) === */                                                                         \
    X(pipeline_creation_failed, "Pipeline state object creation failed", 0x0501, std::errc::invalid_argument)      \
    X(pipeline_state_invalid, "Invalid pipeline state configuration", 0x0502, std::errc::invalid_argument)         \
    X(pipeline_blend_state_invalid, "Invalid blend state", 0x0503, std::errc::invalid_argument)                    \
    X(pipeline_depth_stencil_invalid, "Invalid depth/stencil state", 0x0504, std::errc::invalid_argument)          \
    X(pipeline_vertex_input_invalid, "Invalid vertex input layout", 0x0505, std::errc::invalid_argument)           \
                                                                                                                   \
    /* === Framebuffer (0x0601-0x06FF) === */                                                                      \
    X(framebuffer_incomplete, "Framebuffer incomplete", 0x0601, std::errc::invalid_argument)                       \
    X(framebuffer_attachment_invalid, "Invalid framebuffer attachment", 0x0602, std::errc::invalid_argument)       \
    X(framebuffer_multiple_color_attachments_unsupported, "MRT not supported", 0x0603, std::errc::not_supported)   \
    X(framebuffer_sample_count_mismatch, "Framebuffer sample count mismatch", 0x0604, std::errc::invalid_argument) \
                                                                                                                   \
    /* === Sync/Query (0x0701-0x07FF) === */                                                                       \
    X(sync_creation_failed, "Sync object creation failed", 0x0701, SKL_STDERR_FALLBACK)                            \
    X(sync_wait_timeout, "Sync wait timeout", 0x0702, std::errc::timed_out)                                        \
    X(query_creation_failed, "Query object creation failed", 0x0703, SKL_STDERR_FALLBACK)                          \
    X(query_result_unavailable, "Query result not ready", 0x0704, std::errc::resource_unavailable_try_again)       \
                                                                                                                   \
    /* === Resource (0x0801-0x08FF) === */                                                                         \
    X(resource_in_use, "Resource still in use by GPU", 0x0801, std::errc::device_or_resource_busy)                 \
    X(resource_binding_conflict, "Resource binding conflict", 0x0802, std::errc::invalid_argument)                 \
    X(resource_state_transition_invalid, "Invalid resource state transition", 0x0803, std::errc::invalid_argument) \
                                                                                                                   \
    /* === Driver (0x0901-0x09FF) === */                                                                           \
    X(driver_internal_error, "Driver internal error", 0x0901, SKL_STDERR_FALLBACK)                                 \
    X(driver_version_unsupported, "Driver version too old", 0x0902, std::errc::not_supported)                      \
    X(platform_extension_unsupported, "Required extension not supported", 0x0903, std::errc::not_supported) \
\
    /* === Model (0x0A01-0x0AFF) === */ \
    X(model_load_failed, "Model load failed", 0xA01, std::errc::no_such_file_or_directory) \

    SKL_GRAPHICS_ERROR_CODES(SKL_ERR_X_ENUM)
};
NAMESPACE_GRAPHICS_END
