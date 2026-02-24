// xmake-type: Dynamic
// xmake-deps: path = {"src"}, file = {{"shader.cpp"}} xmake-deps-end
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

public:
    static constexpr size_t SRC_BUFF_SIZE = 4096;
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    Shader(const Shader&) = default;
    Shader(Shader&&) = default;
    Shader& operator=(const Shader&) = default;
    Shader& operator=(Shader&&) = default;

    [[nodiscard]] auto getID() const -> decltype(ID) { return ID; }
    [[nodiscard]] auto getSuccess() const -> decltype(success) { return success; }

    void use() const {
        if (success) glUseProgram(ID);
    }

    void setBool(const char* name, GLboolean value) const { glUniform1i(glGetUniformLocation(ID, name), (GLint)value); }
    void setBool(const char* name, GLsizei count, GLboolean* value) const {
        glUniform1iv(glGetUniformLocation(ID, name), count, (GLint*)value);
    }

// NOLINTBEGIN
#define SetUniformVecFn(fn, glfn, type)                                                                                  \
    void set1##fn(const char* name, type value) const { glUniform1##glfn(glGetUniformLocation(ID, name), value); }    \
    void set1##fn(const char* name, GLsizei count, const type* value) const {                                         \
        glUniform1##glfn##v(glGetUniformLocation(ID, name), count, value);                                            \
    }                                                                                                                 \
    void set2##fn(const char* name, type x, type y) const { glUniform2##glfn(glGetUniformLocation(ID, name), x, y); } \
    void set2##fn(const char* name, GLsizei count, const type* value) const {                                         \
        glUniform2##glfn##v(glGetUniformLocation(ID, name), count, value);                                            \
    }                                                                                                                 \
    void set3##fn(const char* name, type x, type y, type z) const {                                                   \
        glUniform3##glfn(glGetUniformLocation(ID, name), x, y, z);                                                    \
    }                                                                                                                 \
    void set3##fn(const char* name, GLsizei count, const type* value) const {                                         \
        glUniform3##glfn##v(glGetUniformLocation(ID, name), count, value);                                            \
    }                                                                                                                 \
    void set4##fn(const char* name, type x, type y, type z, type w) const {                                           \
        glUniform4##glfn(glGetUniformLocation(ID, name), x, y, z, w);                                                 \
    }                                                                                                                 \
    void set4##fn(const char* name, GLsizei count, const type* value) const {                                         \
        glUniform4##glfn##v(glGetUniformLocation(ID, name), count, value);                                            \
    }

#define SetUniformMatFn(fn, glfn, type)                                                               \
    void setMat2##fn(const char* name, GLsizei count, GLboolean transpose, const type* value) const { \
        glUniformMatrix2##glfn##v(glGetUniformLocation(ID, name), count, transpose, value);           \
    }                                                                                                 \
    void setMat3##fn(const char* name, GLsizei count, GLboolean transpose, const type* value) const { \
        glUniformMatrix3##glfn##v(glGetUniformLocation(ID, name), count, transpose, value);           \
    }                                                                                                 \
    void setMat4##fn(const char* name, GLsizei count, GLboolean transpose, const type* value) const { \
        glUniformMatrix4##glfn##v(glGetUniformLocation(ID, name), count, transpose, value);           \
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


    private : static GLboolean BuildShader(FILE* fp, GLenum type, GLuint* shader);
    static GLboolean checkCompile(unsigned int shader, GLenum type);
};

const char* getTypeName(GLenum type);
}   // namespace skl::opengl
