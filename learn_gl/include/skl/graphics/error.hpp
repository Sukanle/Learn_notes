// SKLGraphicsError.h
#pragma once
#include "skl/error_code.h"
#include <unordered_map>

namespace skl {

#include <cstdint>

enum class graphics_ec : uint16_t {
    // === Common (0x0001 - 0x01FF) ===
    invalid_argument = (uint16_t)std::errc::invalid_argument,   // 参数错误
    unknown = 0x01FF,                                           // 未分类的错误
    // === Device / Context (0x0101 - 0x01FF) ===
    device_lost = 0x0101,                    // GPU 设备丢失 (TDR/休眠/热插拔)
    device_timeout = 0x0102,                 // GPU 操作超时 (未按时完成任务)
    out_of_memory = 0x0103,                  // 显存/视频内存耗尽
    device_initialization_failed = 0x0104,   // 设备/上下文初始化失败
    feature_unsupported = 0x0105,            // 请求的 GPU 特性不支持
    context_lost = 0x0106,                   // 渲染上下文丢失 (需重建资源)

    // === Shader (0x0201 - 0x02FF) ===
    shader_compile_failed = 0x0201,            // 着色器编译错误 (语法/语义)
    shader_link_failed = 0x0202,               // 着色器链接错误 (接口不匹配)
    shader_validation_failed = 0x0203,         // 着色器验证失败 (驱动拒绝)
    shader_entry_point_not_found = 0x0204,     // 找不到入口函数 (如 main)
    shader_source_load_failed = 0x0205,        // 源码文件读取失败
    shader_invalid_format = 0x0206,            // 着色器二进制格式不兼容
    shader_resource_limit_exceeded = 0x0207,   // 超出寄存器/常量缓冲区限制

    // === Texture / Image (0x0301 - 0x03FF) ===
    texture_creation_failed = 0x0301,            // 纹理对象创建失败
    texture_format_unsupported = 0x0302,         // 像素格式/压缩格式不支持
    texture_upload_failed = 0x0303,              // 数据上传到显存失败
    texture_mipmap_generation_failed = 0x0304,   // 自动生成 Mipmap 失败
    texture_boundary_invalid = 0x0305,           // 采样边界模式不支持
    texture_size_exceeded = 0x0306,              // 纹理尺寸超过硬件限制
    texture_array_layer_exceeded = 0x0307,       // 数组纹理层数超限
    texture_unit_exhausted = 0x0308,             // 纹理单元耗尽
    texture_unit_not_bound = 0x0309,            // 查询的位置/单元未绑定有效纹理
    texture_unit_mismatch = 0x030A,             // 位置绑定的纹理ID与预期不符

    // === Buffer / Memory (0x0401 - 0x04FF) ===
    buffer_creation_failed = 0x0401,   // 缓冲区对象创建失败
    buffer_mapping_failed = 0x0402,    // 内存映射 (glMapBuffer) 失败
    buffer_binding_failed = 0x0403,    // 缓冲区绑定到目标失败
    buffer_overflow = 0x0404,          // 写入数据超出缓冲区容量
    buffer_range_invalid = 0x0405,     // 访问的偏移/长度非法

    // === Pipeline / State (0x0501 - 0x05FF) ===
    pipeline_creation_failed = 0x0501,         // 管线对象 (PSO) 创建失败
    pipeline_state_invalid = 0x0502,           // 管线状态配置冲突
    pipeline_blend_state_invalid = 0x0503,     // 混合状态配置非法
    pipeline_depth_stencil_invalid = 0x0504,   // 深度/模板状态配置非法
    pipeline_vertex_input_invalid = 0x0505,    // 顶点输入布局不匹配

    // === Framebuffer / Render Target (0x0601 - 0x06FF) ===
    framebuffer_incomplete = 0x0601,                               // 帧缓冲区不完整 (无法渲染)
    framebuffer_attachment_invalid = 0x0602,                       // 附件格式/维度不兼容
    framebuffer_multiple_color_attachments_unsupported = 0x0603,   // 不支持多渲染目标
    framebuffer_sample_count_mismatch = 0x0604,                    // 附件采样数不一致

    // === Sync / Query (0x0701 - 0x07FF) ===
    sync_creation_failed = 0x0701,       // 同步对象创建失败
    sync_wait_timeout = 0x0702,          // 等待 GPU 同步超时
    query_creation_failed = 0x0703,      // 查询对象创建失败
    query_result_unavailable = 0x0704,   // 查询结果尚未就绪

    // === Resource / General (0x0801 - 0x08FF) ===
    resource_in_use = 0x0801,                     // 资源仍被 GPU 引用，无法销毁/修改
    resource_binding_conflict = 0x0802,           // 资源绑定位置冲突
    resource_state_transition_invalid = 0x0803,   // 资源状态转换非法 (如 UAV->RT)

    // === Driver / Platform (0x0901 - 0x09FF) ===
    driver_internal_error = 0x0901,            // 驱动内部错误 (非应用层问题)
    driver_version_unsupported = 0x0902,       // 驱动版本过旧，不支持所需 API
    platform_extension_unsupported = 0x0903,   // 所需平台扩展未启用

};
/**
 * @brief Graphics 子系统错误类别实现
 */
class graphics_err : public error_category {
public:
    // 单例访问
    static const graphics_err &instance() {
        static graphics_err inst;
        return inst;
    }

    // 必需重写
    const char *name() const noexcept override;
    const char *moduleId() const noexcept override;
    const char *moduleName() const noexcept override;

    std::string message(int ev) const override;

    // 可选：映射到标准错误条件
    std::error_condition default_error_condition(int ev) const noexcept override;

private:
    graphics_err() = default;

    // 错误码 -> 消息映射 (可通过脚本自动生成)
    static auto getErrorMap() -> const std::unordered_map<graphics_ec, const char *> &;
    static std::string toHex(int val);
};

std::error_code make_error_code(graphics_ec ec);

}   // namespace skl
