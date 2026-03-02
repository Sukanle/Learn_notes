#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifdef __MACRO_UNUSED__
#define __GL_UNUSED(var) ((void)var)
#define __GL_UNUSED_RET(fn) ((void)fn)
#endif

namespace skl::opengl {
class Shader {
private:
    GLuint ID;
    GLboolean success;
    GLboolean BuildShader(FILE* fp, GLenum type, GLuint* shader) noexcept;
    GLboolean checkCompile(unsigned int shader, GLenum type) noexcept;

public:
    static constexpr size_t SRC_BUFF_SIZE = 4096;
    Shader(const char* vertexPath, const char* fragmentPath) noexcept;
    ~Shader();

    Shader(const Shader&) noexcept = default;
    Shader(Shader&&) noexcept = default;
    Shader& operator=(const Shader&) noexcept = default;
    Shader& operator=(Shader&&) noexcept = default;

    [[nodiscard]] auto getID() const noexcept -> decltype(ID) { return ID; }
    [[nodiscard]] auto getSuccess() const noexcept -> decltype(success) { return success; }

    void use() const {
        if (success) glUseProgram(ID);
    }

    void setBool(const char* name, GLboolean value) const noexcept {
        glUniform1i(glGetUniformLocation(ID, name), (GLint)value);
    }
    void setBool(const char* name, GLsizei count, GLboolean* value) const noexcept {
        glUniform1iv(glGetUniformLocation(ID, name), count, (GLint*)value);
    }

// NOLINTBEGIN
#define SetUniformVecFn(fn, glfn, type)                                                \
    void set1##fn(const char* name, type value) const noexcept {                       \
        glUniform1##glfn(glGetUniformLocation(ID, name), value);                       \
    }                                                                                  \
    void set1##fn(const char* name, GLsizei count, const type* value) const noexcept { \
        glUniform1##glfn##v(glGetUniformLocation(ID, name), count, value);             \
    }                                                                                  \
    void set2##fn(const char* name, type x, type y) const noexcept {                   \
        glUniform2##glfn(glGetUniformLocation(ID, name), x, y);                        \
    }                                                                                  \
    void set2##fn(const char* name, GLsizei count, const type* value) const noexcept { \
        glUniform2##glfn##v(glGetUniformLocation(ID, name), count, value);             \
    }                                                                                  \
    void set3##fn(const char* name, type x, type y, type z) const noexcept {           \
        glUniform3##glfn(glGetUniformLocation(ID, name), x, y, z);                     \
    }                                                                                  \
    void set3##fn(const char* name, GLsizei count, const type* value) const noexcept { \
        glUniform3##glfn##v(glGetUniformLocation(ID, name), count, value);             \
    }                                                                                  \
    void set4##fn(const char* name, type x, type y, type z, type w) const noexcept {   \
        glUniform4##glfn(glGetUniformLocation(ID, name), x, y, z, w);                  \
    }                                                                                  \
    void set4##fn(const char* name, GLsizei count, const type* value) const noexcept { \
        glUniform4##glfn##v(glGetUniformLocation(ID, name), count, value);             \
    }

#define SetUniformMatFn(fn, glfn, type)                                                                        \
    void setMat2##fn(const char* name, GLsizei count, GLboolean transpose, const type* value) const noexcept { \
        glUniformMatrix2##glfn##v(glGetUniformLocation(ID, name), count, transpose, value);                    \
    }                                                                                                          \
    void setMat3##fn(const char* name, GLsizei count, GLboolean transpose, const type* value) const noexcept { \
        glUniformMatrix3##glfn##v(glGetUniformLocation(ID, name), count, transpose, value);                    \
    }                                                                                                          \
    void setMat4##fn(const char* name, GLsizei count, GLboolean transpose, const type* value) const noexcept { \
        glUniformMatrix4##glfn##v(glGetUniformLocation(ID, name), count, transpose, value);                    \
    }
    // NOLINTEND
    // clang-format off
    SetUniformVecFn(I, i, GLint)
    SetUniformVecFn(Ui, ui, GLuint)
    SetUniformVecFn(F, f, GLfloat)
    SetUniformVecFn(D, d, GLdouble)

    SetUniformMatFn(F, f, GLfloat)
    SetUniformMatFn(D, d, GLdouble)
    // clang-format on
#undef SetUniformVecFn
#undef SetUniformMatFn
};

const char* getTypeName(GLenum type);
}   // namespace skl::opengl
