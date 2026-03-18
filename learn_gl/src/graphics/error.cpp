#include "skl/graphics/error.hpp"
#include <format>

namespace skl {

const char *graphics_err::name() const noexcept {
    return "skl.graphics";
}
const char *graphics_err::moduleId() const noexcept {
    return "graphics";
}
const char *graphics_err::moduleName() const noexcept {
    return "Graphics Subsystem";
}
std::string graphics_err::message(int ev) const {
    // 使用静态映射表，避免大量 switch
    auto map = getErrorMap();
    auto it = map.find((graphics_ec)ev);
    if (it != map.end()) return it->second;
    return std::format("Undefined graphics error: {:X}", ev);
}


std::error_condition graphics_err::default_error_condition(int ev) const noexcept {
    // 示例：将显存不足映射到标准条件
    // if (ev == 0x003) return std::errc::not_enough_memory;   // GL_OUT_OF_MEMORY
    // if (ev == 0x101) return std::errc::invalid_argument;    // GL_INVALID_ENUM
    // return {ev, *this};
    if (!ev) return {0, std::generic_category()};
    // 按模块映射到最接近的 std::errc
    switch (static_cast<graphics_ec>(ev)) {
        case skl::graphics_ec::invalid_argument:        return std::errc::invalid_argument;
        // === Device (0x01xx) ===
        case graphics_ec::out_of_memory:                return std::errc::not_enough_memory;
        case graphics_ec::device_timeout:               return std::errc::timed_out;
        // === Shader (0x01xx) ===
        case graphics_ec::shader_compile_failed:
        case graphics_ec::shader_link_failed:
        case graphics_ec::shader_validation_failed:     return std::errc::invalid_argument;
        case graphics_ec::shader_source_load_failed:    return std::errc::io_error;
        case graphics_ec::shader_entry_point_not_found: return std::errc::invalid_argument;
        // === Texture (0x02xx) ===
        case graphics_ec::texture_format_unsupported:
        case graphics_ec::texture_creation_failed:
        case graphics_ec::texture_unit_not_bound:
        case graphics_ec::texture_unit_mismatch:        return std::errc::invalid_argument;
        case graphics_ec::texture_upload_failed:        return std::errc::io_error;
        // === Buffer (0x03xx) ===
        case graphics_ec::buffer_creation_failed:       return std::errc::invalid_argument;
        case graphics_ec::buffer_mapping_failed:        return std::errc::io_error;
        case graphics_ec::buffer_overflow:              return std::errc::invalid_argument;
        // === Pipeline/State (0x04xx) ===
        case graphics_ec::pipeline_state_invalid:
        case graphics_ec::pipeline_creation_failed:     return std::errc::invalid_argument;
        // === Framebuffer (0x05xx) ===
        case graphics_ec::framebuffer_incomplete:       return std::errc::invalid_argument;
        default:                                        return {ev, *this};
    }
}

auto graphics_err::getErrorMap() -> const std::unordered_map<graphics_ec, const char *> & {
    static const std::unordered_map<graphics_ec, const char *> map = {
        // === Common (0x0001-0x00FF) ===
        {graphics_ec::invalid_argument, "Graphics Invalid argument"},
        {graphics_ec::unknown, "Unknown graphics error"},

        // === Device (0x0101-0x01FF) ===
        {graphics_ec::device_lost, "GPU device lost"},
        {graphics_ec::device_timeout, "GPU operation timed out"},
        {graphics_ec::out_of_memory, "Out of GPU memory"},
        {graphics_ec::device_initialization_failed, "Device initialization failed"},
        {graphics_ec::feature_unsupported, "Required GPU feature not supported"},
        {graphics_ec::context_lost, "Rendering context lost"},

        // === Shader (0x0201-0x02FF) ===
        {graphics_ec::shader_compile_failed, "Shader compilation failed"},
        {graphics_ec::shader_link_failed, "Shader linking failed"},
        {graphics_ec::shader_validation_failed, "Shader validation failed"},
        {graphics_ec::shader_entry_point_not_found, "Shader entry point not found"},
        {graphics_ec::shader_source_load_failed, "Failed to load shader source file"},
        {graphics_ec::shader_invalid_format, "Invalid shader binary format"},
        {graphics_ec::shader_resource_limit_exceeded, "Shader resource limit exceeded"},

        // === Texture (0x0301-0x03FF) ===
        {graphics_ec::texture_creation_failed, "Texture creation failed"},
        {graphics_ec::texture_format_unsupported, "Texture format not supported"},
        {graphics_ec::texture_upload_failed, "Texture data upload failed"},
        {graphics_ec::texture_mipmap_generation_failed, "Mipmap generation failed"},
        {graphics_ec::texture_boundary_invalid, "Invalid texture boundary mode"},
        {graphics_ec::texture_size_exceeded, "Texture size exceeds hardware limit"},
        {graphics_ec::texture_array_layer_exceeded, "Texture array layers exceeded"},
        {graphics_ec::texture_unit_exhausted, "No available texture image units"},
        {graphics_ec::texture_unit_not_bound, "Texture unit not bound to valid texture"},
        {graphics_ec::texture_unit_mismatch, "Texture unit bound to unexpected texture ID"},

        // === Buffer (0x0401-0x04FF) ===
        {graphics_ec::buffer_creation_failed, "Buffer allocation failed"},
        {graphics_ec::buffer_mapping_failed, "Buffer memory mapping failed"},
        {graphics_ec::buffer_binding_failed, "Buffer binding failed"},
        {graphics_ec::buffer_overflow, "Buffer write overflow"},
        {graphics_ec::buffer_range_invalid, "Invalid buffer access range"},

        // === Pipeline (0x0501-0x05FF) ===
        {graphics_ec::pipeline_creation_failed, "Pipeline state object creation failed"},
        {graphics_ec::pipeline_state_invalid, "Invalid pipeline state configuration"},
        {graphics_ec::pipeline_blend_state_invalid, "Invalid blend state"},
        {graphics_ec::pipeline_depth_stencil_invalid, "Invalid depth/stencil state"},
        {graphics_ec::pipeline_vertex_input_invalid, "Invalid vertex input layout"},
        // === Framebuffer (0x0601-0x06FF) ===
        {graphics_ec::framebuffer_incomplete, "Framebuffer incomplete"},
        {graphics_ec::framebuffer_attachment_invalid, "Invalid framebuffer attachment"},
        {graphics_ec::framebuffer_multiple_color_attachments_unsupported, "MRT not supported"},
        {graphics_ec::framebuffer_sample_count_mismatch, "Framebuffer sample count mismatch"},

        // === Sync/Query (0x0701-0x07FF) ===
        {graphics_ec::sync_creation_failed, "Sync object creation failed"},
        {graphics_ec::sync_wait_timeout, "Sync wait timeout"},
        {graphics_ec::query_creation_failed, "Query object creation failed"},
        {graphics_ec::query_result_unavailable, "Query result not ready"},

        // === Resource (0x0801-0x08FF) ===
        {graphics_ec::resource_in_use, "Resource still in use by GPU"},
        {graphics_ec::resource_binding_conflict, "Resource binding conflict"},
        {graphics_ec::resource_state_transition_invalid, "Invalid resource state transition"},

        // === Driver (0x0901-0x09FF) ===
        {graphics_ec::driver_internal_error, "Driver internal error"},
        {graphics_ec::driver_version_unsupported, "Driver version too old"},
        {graphics_ec::platform_extension_unsupported, "Required extension not supported"},
    };
    return map;
}
std::error_code make_error_code(graphics_ec ec) {
    return {(int)ec, graphics_err::instance()};
}
}   // namespace skl
