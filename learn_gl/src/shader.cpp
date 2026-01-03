#define __MACRO_UNUSED__
#include "skl/base.hpp"
#include "skl/shader.hpp"

namespace skl::opengl {
static char srcbuff[Shader::SRC_BUFF_SIZE];
char* StaticBuf(FILE* fp, size_t n) {
    fread(srcbuff, n, 1, fp);
    srcbuff[n + 1] = '\0';
    return srcbuff;
}
char* DynamicBuf(FILE* fp, size_t n) {
    char* buff = (char*)malloc(aligned64(n + 1));
    fread(buff, n, 1, fp);
    buff[n + 1] = '\0';
    return buff;
}
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

GLboolean Shader::checkCompile(unsigned int shader, GLenum type) {
    int success = 0;
    char infoLog[1024] = {};
    if (type == GL_FALSE) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            __GL_UNUSED_RET(fprintf(stderr,
                                    "ERROR::SHADER_COMPILATION_ERROR of type: %s\n"
                                    "%s\n"
                                    " -- --------------------------------------------------- -- \n",
                                    getTypeName(type), infoLog));
            return GL_FALSE;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            __GL_UNUSED_RET(fprintf(stderr,
                                    "ERROR::PROGRAM_LINKING_ERROR of type: %s\n"
                                    "%s\n"
                                    " -- --------------------------------------------------- -- \n",
                                    getTypeName(type), infoLog));
            return GL_FALSE;
        }
    }
    return GL_TRUE;
}
GLboolean Shader::BuildShader(FILE* fp, GLenum type, GLuint* shader) {
    size_t n = fsize(fp);
    if (!n) return GL_FALSE;
    GLboolean isStatic = (n < SRC_BUFF_SIZE - 1);
    char* src = isStatic ? StaticBuf(fp, n) : DynamicBuf(fp, n);
    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &src, NULL);
    glCompileShader(*shader);
    if (!isStatic) free(src);
    if (!checkCompile(*shader, type)) { return GL_FALSE; }
    return GL_TRUE;
}
Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    FILE* vertexSrc = fopen(vertexPath, "rb");
    FILE* fragmentSrc = fopen(fragmentPath, "rb");
    GLuint vertex = 0;
    GLuint fragment = 0;
    success = BuildShader(vertexSrc, GL_VERTEX_SHADER, &vertex);
    success &= BuildShader(fragmentSrc, GL_FRAGMENT_SHADER, &fragment);
    if (!success) return;
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
