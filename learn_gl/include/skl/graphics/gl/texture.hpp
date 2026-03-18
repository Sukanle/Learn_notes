#pragma once

#include <glad/glad.h>
#include "skl/base.hpp"
#include <array>
#include <string>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <filesystem>

namespace skl::opengl {
// using TexParam = std::variant<GLint, GLfloat, GLint[4], GLfloat[4]>;
struct TexParam {
public:
    enum Tparam : uint8_t { I1, F1, I4, F4 };
    void set(Tparam tname, const void *data) noexcept;
    void set(GLint value) noexcept;
    void set(GLfloat value) noexcept;
    void set(GLint *value) noexcept;
    void set(GLfloat *value) noexcept;
    void set(std::array<GLint, 4> value) noexcept;
    void set(std::array<GLfloat, 4> value) noexcept;

    union {
        GLint i;
        GLfloat f;
        GLint iv[4];
        GLfloat fv[4];
    } data;
    Tparam type;

    constexpr bool operator==(const TexParam &other) const noexcept;
    constexpr bool operator!=(const TexParam &other) const noexcept;
};
struct texture_t {
    GLuint ID = 0;
    GLint width = 0;
    GLint height = 0;
    GLint nrChannels = 0;
    GLint mipLevels = 1;
    GLenum type = GL_FALSE;
    GLenum intformat = GL_FALSE;
    GLvoid *data = nullptr;
    std::string path;

    texture_t() = default;
    explicit texture_t(std::string_view texpath, GLenum intformat, GLenum type, GLint mipLevels) noexcept;
};

struct TexConfig {
    TexParam param;
    GLenum pname;
    bool dirty;

    constexpr bool operator==(const TexConfig &other) const noexcept;
    constexpr bool operator!=(const TexConfig &other) const noexcept;
};

class Texture2D {
private:
    texture_t _tex;
    std::vector<TexConfig> _cfgs;
    static GLuint _bind_id;
    bool _dirty;
    bool _autoUnit;
    int _unitPos;
    void apply(GLenum pname, const TexParam &param) const noexcept;
    void apply(TexConfig &cfg) const noexcept {
        apply(cfg.pname, cfg.param);
        cfg.dirty = false;
    }

public:
    Texture2D() noexcept;
    explicit Texture2D(const std::filesystem::path &tex, GLenum intformat = GL_FALSE, GLenum type = GL_UNSIGNED_BYTE,
                       GLint mipLevels = 1) noexcept;
    ~Texture2D() noexcept;

    static GLint GetGLFormat(int channels)noexcept;
    static void setflipY(bool flip) noexcept {
        stbi_set_flip_vertically_on_load(flip);
    }
    Texture2D &bind() noexcept;

    Texture2D &load(std::error_code &ec, const std::filesystem::path &tex, GLint intformat = GL_FALSE,
                    GLenum type = GL_UNSIGNED_BYTE, GLenum mipLevels = 1) noexcept;
    template<typename T>
    Texture2D &set_config(GLenum pname, T value) noexcept;
    Texture2D &set_config(UpdateMode mode, const std::vector<TexConfig> &cfgs) noexcept;
    Texture2D &set_config(UpdateMode policy, std::initializer_list<TexConfig> cfgs) noexcept;
    Texture2D &update() noexcept;
    Texture2D &activate(std::error_code &ec) noexcept;
    Texture2D &activate(std::error_code &ec, int pos) noexcept;
};

class TextureUnit {
public:
    static TextureUnit &instance() noexcept;
    int acquire(std::error_code &ec) noexcept;
    void release(std::error_code &ec, int unit, int pos) noexcept;
    void activate(std::error_code &ec, int unit, int pos) noexcept;

private:
    int _maxUnits;
    int _activeUnit;
    std::vector<int> _units;
    TextureUnit() noexcept;
};
}   // namespace skl::opengl
