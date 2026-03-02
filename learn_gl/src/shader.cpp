#define __MACRO_UNUSED__
#include "skl/base.hpp"
#include "skl/shader.hpp"

#include <errno.h>

namespace skl::opengl {
static char srcbuff[Shader::SRC_BUFF_SIZE];
/**
 * 获取文件大小（跨平台）
 * @param fp 文件指针
 * @return 文件大小（字节），失败时返回0
 */
size_t fsize(FILE* fp) {
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

GLboolean ReadFileToBuffer(FILE* fp, char** out_src, size_t* out_len, GLboolean* out_isStatic) {
    if (!fp || !out_src || !out_len || !out_isStatic) return GL_FALSE;

    if (fseek(fp, 0, SEEK_SET) != 0) return GL_FALSE;
    size_t n = fsize(fp);
    if (!n) return GL_FALSE;

    if (fseek(fp, 0, SEEK_SET) != 0) return GL_FALSE;

    if (n < Shader::SRC_BUFF_SIZE) {
        size_t read_size = fread(srcbuff, 1, n, fp);
        if (read_size != n) return GL_FALSE;
        srcbuff[n] = '\0';

        *out_src = srcbuff;
        *out_len = n;
        *out_isStatic = GL_TRUE;
        return GL_TRUE;
    }

    char* heap_buf = (char*)malloc(n + 1);
    if (!heap_buf) return GL_FALSE;

    size_t read_size = fread(heap_buf, 1, n, fp);
    if (read_size != n) {
        free(heap_buf);
        return GL_FALSE;
    }

    heap_buf[n] = '\0';
    *out_src = heap_buf;
    *out_len = n;
    *out_isStatic = GL_FALSE;
    return GL_TRUE;
}

/**
 * 将GLenum类型转换为可读字符串
 */
const char* getTypeName(GLenum type) {
    switch (type) {
        case GL_VERTEX_SHADER:   return "VERTEX_SHADER";
        case GL_FRAGMENT_SHADER: return "FRAGMENT_SHADER";
        case GL_GEOMETRY_SHADER: return "GEOMETRY_SHADER";
        case GL_FALSE:           return "PROGRAM";
        default:                 return "UNKNOWN";
    }
}
GLboolean Shader::checkCompile(unsigned int shader, GLenum type) noexcept{
    int success = 0;
    char infoLog[1024] = {};
    if (type == GL_FALSE) {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            __GL_UNUSED_RET(fprintf(stderr,
                                    "ERROR::PROGRAM_LINKING_ERROR of type:"
                                    "%s\n"
                                    "%s\n"
                                    " --"
                                    "---------------------------------------------------"
                                    "-- \n",
                                    getTypeName(type), infoLog));
            return GL_FALSE;
        }
    } else {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            __GL_UNUSED_RET(fprintf(stderr,
                                    "ERROR::SHADER_COMPILATION_ERROR of"
                                    " type: %s\n"
                                    "%s\n"
                                    " --"
                                    "---------------------------------------------------"
                                    "-- \n",
                                    getTypeName(type), infoLog));
            return GL_FALSE;
        }
    }
    return GL_TRUE;
}

GLboolean Shader::BuildShader(FILE* fp, GLenum type, GLuint* shader) noexcept{
    if (!fp || !shader) return GL_FALSE;

    char* src = NULL;
    size_t src_size = 0;
    GLboolean isStatic = GL_FALSE;
    if (!ReadFileToBuffer(fp, &src, &src_size, &isStatic)) return GL_FALSE;

    *shader = glCreateShader(type);
    GLint src_length = (GLint)src_size;
    glShaderSource(*shader, 1, &src, &src_length);
    glCompileShader(*shader);
    if (!isStatic) free(src);
    if (!checkCompile(*shader, type)) {
        glDeleteShader(*shader);
        *shader = 0;
        return GL_FALSE;
    }
    return GL_TRUE;
}
Shader::Shader(const char* vertexPath, const char* fragmentPath) noexcept{
    ID = success = GL_FALSE;

    FILE* vertexSrc = fopen(vertexPath, "rb");
    int vertexErrno = errno;
    FILE* fragmentSrc = fopen(fragmentPath, "rb");
    int fragmentErrno = errno;
    if (!vertexSrc || !fragmentSrc) {
        if (vertexSrc) fclose(vertexSrc);
        if (fragmentSrc) fclose(fragmentSrc);
        __GL_UNUSED_RET(fprintf(stderr,
                                "ERROR::SHADER_FILE_OPEN_ERROR\n"
                                "vertex: %s (%s)\n"
                                "fragment: %s (%s)\n"
                                " --"
                                "---------------------------------------------------"
                                "-- \n",
                                vertexPath ? vertexPath : "(null)", strerror(vertexErrno),
                                fragmentPath ? fragmentPath : "(null)", strerror(fragmentErrno)));
        return;
    }

    GLuint vertex = 0;
    GLuint fragment = 0;
    success = BuildShader(vertexSrc, GL_VERTEX_SHADER, &vertex);
    success &= BuildShader(fragmentSrc, GL_FRAGMENT_SHADER, &fragment);

    fclose(vertexSrc);
    fclose(fragmentSrc);

    if (!success) {
        if (vertex) glDeleteShader(vertex);
        if (fragment) glDeleteShader(fragment);
        return;
    }

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    success &= checkCompile(ID, GL_FALSE);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}
Shader::~Shader() {
    if (success && ID) {
        glDeleteProgram(ID);
        ID = 0;
        success = GL_FALSE;
    }
}
}   // namespace skl::opengl
