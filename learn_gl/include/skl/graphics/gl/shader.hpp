#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include "skl/base.hpp"
#ifdef __MACRO_UNUSED__
#define __GL_UNUSED(var) ((void)var)
#define __GL_UNUSED_RET(fn) ((void)fn)
#endif

namespace skl::opengl {
class Shader {
private:
    GLuint _ID;
    SKLErr BuildShader(FILE *fp, GLenum type, GLuint &shader) noexcept;
    SKLErr checkCompile(unsigned int shader, GLenum type) noexcept;

public:
    static constexpr size_t SRC_BUFF_SIZE = 4096;
    Shader()noexcept:_ID(0) {};
    ~Shader()noexcept;

    Shader(const Shader &) noexcept = default;
    Shader(Shader &&) noexcept = default;
    Shader &operator=(const Shader &) noexcept = default;
    Shader &operator=(Shader &&) noexcept = default;

    Shader& build(std::error_code &ec, const char *vertexPath, const char *fragmentPath)noexcept;
    Shader& build(std::error_code &ec, const std::filesystem::path &vertexPath, const std::filesystem::path& fragmentPath)noexcept;
    [[nodiscard]] auto getID() const noexcept -> decltype(_ID) { return _ID; }

    void use() const {
        glUseProgram(_ID);
    }

    void setBool(const char *name, GLboolean value) const noexcept {
        glUniform1i(glGetUniformLocation(_ID, name), (GLint)value);
    }
    void setBool(const char *name, GLsizei count, GLboolean *value) const noexcept {
        glUniform1iv(glGetUniformLocation(_ID, name), count, (GLint *)value);
    }

// NOLINTBEGIN
#define SetUniformVecFn(fn, glfn, type)                                                \
    void set1##fn(const char *name, type value) const noexcept {                       \
        glUniform1##glfn(glGetUniformLocation(_ID, name), value);                       \
    }                                                                                  \
    void set1##fn(const char *name, GLsizei count, const type *value) const noexcept { \
        glUniform1##glfn##v(glGetUniformLocation(_ID, name), count, value);             \
    }                                                                                  \
    void set2##fn(const char *name, type x, type y) const noexcept {                   \
        glUniform2##glfn(glGetUniformLocation(_ID, name), x, y);                        \
    }                                                                                  \
    void set2##fn(const char *name, GLsizei count, const type *value) const noexcept { \
        glUniform2##glfn##v(glGetUniformLocation(_ID, name), count, value);             \
    }                                                                                  \
    void set3##fn(const char *name, type x, type y, type z) const noexcept {           \
        glUniform3##glfn(glGetUniformLocation(_ID, name), x, y, z);                     \
    }                                                                                  \
    void set3##fn(const char *name, GLsizei count, const type *value) const noexcept { \
        glUniform3##glfn##v(glGetUniformLocation(_ID, name), count, value);             \
    }                                                                                  \
    void set4##fn(const char *name, type x, type y, type z, type w) const noexcept {   \
        glUniform4##glfn(glGetUniformLocation(_ID, name), x, y, z, w);                  \
    }                                                                                  \
    void set4##fn(const char *name, GLsizei count, const type *value) const noexcept { \
        glUniform4##glfn##v(glGetUniformLocation(_ID, name), count, value);             \
    }

#define SetUniformMatFn(fn, glfn, type)                                                                        \
    void setMat2##fn(const char *name, GLsizei count, GLboolean transpose, const type *value) const noexcept { \
        glUniformMatrix2##glfn##v(glGetUniformLocation(_ID, name), count, transpose, value);                    \
    }                                                                                                          \
    void setMat3##fn(const char *name, GLsizei count, GLboolean transpose, const type *value) const noexcept { \
        glUniformMatrix3##glfn##v(glGetUniformLocation(_ID, name), count, transpose, value);                    \
    }                                                                                                          \
    void setMat4##fn(const char *name, GLsizei count, GLboolean transpose, const type *value) const noexcept { \
        glUniformMatrix4##glfn##v(glGetUniformLocation(_ID, name), count, transpose, value);                    \
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

const char *getTypeName(GLenum type);
}   // namespace skl::opengl
