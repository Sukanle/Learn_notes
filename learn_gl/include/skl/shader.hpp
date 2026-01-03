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
    void setBoolv(const char* name, GLsizei count, GLboolean* value) const {
        glUniform1iv(glGetUniformLocation(ID, name), count, (GLint*)value);
    }

    void setInt(const char* name, GLint value) const { glUniform1i(glGetUniformLocation(ID, name), value); }
    void setIntv(const char* name, GLsizei count, GLint* value) const {
        glUniform1iv(glGetUniformLocation(ID, name), count, value);
    }

    void setFloat(const char* name, GLfloat value) const { glUniform1f(glGetUniformLocation(ID, name), value); }
    void setFloatv(const char* name, GLsizei count, GLfloat* value) const {
        glUniform1fv(glGetUniformLocation(ID, name), count, value);
    }

private:
    static GLboolean BuildShader(FILE* fp, GLenum type, GLuint* shader);
    static GLboolean checkCompile(unsigned int shader, GLenum type);
};

const char* getTypeName(GLenum type);
}   // namespace skl::opengl
