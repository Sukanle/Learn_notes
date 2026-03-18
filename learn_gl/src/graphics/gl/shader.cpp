#define __MACRO_UNUSED__
#include "skl/graphics/error.hpp"
#include "skl/graphics/gl/shader.hpp"

#include <errno.h>

namespace skl::opengl {
static char srcbuff[Shader::SRC_BUFF_SIZE];
/**
 * 获取文件大小（跨平台）
 * @param fp 文件指针
 * @return 文件大小（字节），失败时返回0
 */
size_t fsize(FILE *fp) {
    if (!fp) return 0;

    long current_pos = ftell(fp);
    if (current_pos == -1) return 0;

    if (fseek(fp, 0, SEEK_END) != 0) {
        fseek(fp, current_pos, SEEK_SET);
        return 0;
    }

    long file_size = ftell(fp);
    fseek(fp, current_pos, SEEK_SET);

    return (file_size > 0) ? (size_t)file_size : 0;
}

SKLErr ReadFileToBuffer(FILE *fp, char **out_src, size_t *out_len, GLboolean *out_isStatic) {
    if (!fp || !out_src || !out_len || !out_isStatic) return (SKLErr)std::errc::invalid_argument;

    if (fseek(fp, 0, SEEK_SET) != 0) return (SKLErr)std::errc::io_error;
    size_t n = fsize(fp);
    if (!n) return GL_FALSE;

    if (fseek(fp, 0, SEEK_SET) != 0) return (SKLErr)std::errc::io_error;

    if (n < Shader::SRC_BUFF_SIZE) {
        size_t read_size = fread(srcbuff, 1, n, fp);
        if (read_size != n) return (SKLErr)std::errc::io_error;
        srcbuff[n] = '\0';

        *out_src = srcbuff;
        *out_len = n;
        *out_isStatic = GL_TRUE;
        return EXIT_SUCCESS;
    }

    char *heap_buf = (char *)malloc(n + 1);
    if (!heap_buf) return (SKLErr)std::errc::not_enough_memory;

    size_t read_size = fread(heap_buf, 1, n, fp);
    if (read_size != n) {
        free(heap_buf);
        return (SKLErr)std::errc::io_error;
    }

    heap_buf[n] = '\0';
    *out_src = heap_buf;
    *out_len = n;
    *out_isStatic = GL_FALSE;
    return EXIT_SUCCESS;
}

/**
 * 将GLenum类型转换为可读字符串
 */
const char *getTypeName(GLenum type) {
    switch (type) {
        case GL_VERTEX_SHADER:   return "VERTEX_SHADER";
        case GL_FRAGMENT_SHADER: return "FRAGMENT_SHADER";
        case GL_GEOMETRY_SHADER: return "GEOMETRY_SHADER";
        case GL_FALSE:           return "PROGRAM";
        default:                 return "UNKNOWN";
    }
}
SKLErr Shader::checkCompile(unsigned int shader, GLenum type) noexcept {
    int success = 0;
    if (type == GL_FALSE) {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) return (SKLErr)graphics_ec::shader_link_failed;
    } else {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) return (SKLErr)graphics_ec::shader_compile_failed;
    }

    return EXIT_SUCCESS;
}

SKLErr Shader::BuildShader(FILE *fp, GLenum type, GLuint &shader) noexcept {
    if (!fp) return GL_FALSE;
    char *src = nullptr;
    size_t src_size = 0;
    GLboolean isStatic = GL_FALSE;
    if (ReadFileToBuffer(fp, &src, &src_size, &isStatic)) { return (SKLErr)graphics_ec::shader_source_load_failed; }
    shader = glCreateShader(type);
    GLint src_length = (GLint)src_size;
    glShaderSource(shader, 1, &src, &src_length);
    glCompileShader(shader);
    if (!isStatic) free(src);
    SKLErr ec = checkCompile(shader, type);
    if (ec) {
        glDeleteShader(shader);
        shader = 0;
        return ec;
    }
    return EXIT_SUCCESS;
}
Shader &Shader::build(std::error_code &ec, const std::filesystem::path &vertexPath,
                      const std::filesystem::path &fragmentPath) noexcept {
    ec.clear();

    if (vertexPath.empty() || fragmentPath.empty()) {
        ec = make_error_code(graphics_ec::invalid_argument);
        return *this;
    }

    bool Exist = std::filesystem::exists(vertexPath, ec);
    if(ec || !std::filesystem::is_regular_file(vertexPath, ec)) return *this;
    Exist &= std::filesystem::exists(fragmentPath, ec);
    if(ec || !std::filesystem::is_regular_file(fragmentPath, ec)) return *this;
    if (!Exist){
        ec = make_error_code(graphics_ec::shader_source_load_failed);
        return *this;
    }

    return build(ec, vertexPath.c_str(), fragmentPath.c_str());
}
Shader &Shader::build(std::error_code &ec, const char *vertexPath, const char *fragmentPath) noexcept {
    ec.clear();
    FILE *vertexSrc = fopen(vertexPath, "rb");
    FILE *fragmentSrc = fopen(fragmentPath, "rb");
    if (!vertexSrc || !fragmentSrc) {
        if (vertexSrc) fclose(vertexSrc);
        if (fragmentSrc) fclose(fragmentSrc);
        ec = make_error_code(graphics_ec::shader_source_load_failed);
        return *this;
    }

    GLuint vertex = 0;
    GLuint fragment = 0;
    ec = make_error_code((graphics_ec)BuildShader(vertexSrc, GL_VERTEX_SHADER, vertex));
    if (ec) goto CLEANUP;
    ec = make_error_code((graphics_ec)BuildShader(fragmentSrc, GL_FRAGMENT_SHADER, fragment));
    if (ec) goto CLEANUP;

    _ID = glCreateProgram();
    glAttachShader(_ID, vertex);
    glAttachShader(_ID, fragment);
    glLinkProgram(_ID);
    ec = make_error_code((graphics_ec)checkCompile(_ID, GL_FALSE));
    if (ec) {
        glDeleteProgram(_ID);
        _ID = 0;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

CLEANUP:
    fclose(fragmentSrc);
    fclose(vertexSrc);
    return *this;
}
Shader::~Shader() noexcept{
    if (_ID) glDeleteProgram(_ID);
}
}   // namespace skl::opengl
