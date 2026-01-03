#pragma once

#include <glad/glad.h>
#include <string>

namespace skl::opengl {
struct texture_t {
    GLuint ID;
    GLint width;
    GLint height;
    GLint nrChannels;
    GLubyte* data;
    std::string path;
};
}   // namespace skl::opengl
