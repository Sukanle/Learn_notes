#pragma once

#include <array>
#include <filesystem>
#include <stdint.h>
#include <string>
#include <vector>

#include <glad/glad.h>

#include "skl/graphics/gl/base.hpp"
#include "skl/graphics/texture.hpp"

NAMESPACE_OPENGL_BEGIN
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
        std::array<GLint, 4> iv;
        std::array<GLfloat, 4> fv;
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
    std::string name;

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

class Texture2D : public ITexture2D, IOpenGLAPI {
private:
        texture_t _tex;
        std::vector<TexConfig> _cfgs;
        static GLuint _bind_id;
        bool _dirty;
        size_t _unitPos;
    static void apply(GLenum pname, const TexParam &param) noexcept;
    static void apply(TexConfig &cfg) noexcept {
        apply(cfg.pname, cfg.param);
        cfg.dirty = false;
    }

public:
    Texture2D() noexcept;
    explicit Texture2D(const std::filesystem::path &tex, GLenum intformat = GL_FALSE, GLenum type = GL_UNSIGNED_BYTE,
                       GLint mipLevels = 1) noexcept;
    ~Texture2D() noexcept override;

    SKL_DELETE_COPYABLE(Texture2D);
    Texture2D(Texture2D &&) noexcept;
    Texture2D &operator=(Texture2D &&) noexcept;

    static GLint getGLformat(int channels) noexcept;

    [[nodiscard]] auto getUnitPos() const noexcept -> size_t { return _unitPos; }
    [[nodiscard]] auto getTex() const noexcept -> const decltype(_tex) & { return _tex; }
    [[nodiscard]] auto getCfgs() const noexcept -> const decltype(_cfgs) & { return _cfgs; }
    [[nodiscard]] auto getKey() const noexcept -> decltype(_tex.ID) { return _tex.ID; }
    Texture2D &bind() noexcept override;
    Texture2D &load(std::error_code &ec, const std::filesystem::path &tex, GLint intformat = GL_FALSE,
                    GLenum type = GL_UNSIGNED_BYTE, GLenum mipLevels = 1) noexcept;

    Texture2D &set_texName(std::string_view name) noexcept {
        _tex.name = name;
        return *this;
    }
    template<typename T>
    Texture2D &set_config(GLenum pname, T value) noexcept;
    Texture2D &set_config(const std::vector<TexConfig> &cfgs, UpdateMode policy = UpdateMode::append) noexcept;
    Texture2D &set_config(std::initializer_list<TexConfig> cfgs, UpdateMode policy = UpdateMode::append) noexcept;
    Texture2D &update() noexcept;
    Texture2D &acquire(std::error_code &ec, size_t pos = SIZE_T_MAX) noexcept;
    Texture2D &release(std::error_code &ec) noexcept;
    Texture2D &activate(std::error_code &ec) noexcept;
};

class TextureUnit {
public:
    static TextureUnit &instance() noexcept;
    [[nodiscard]] size_t acquire(std::error_code &ec, GLuint TexID, size_t pos = SIZE_T_MAX) noexcept;
    void release(std::error_code &ec, size_t pos) noexcept;
    void activate(std::error_code &ec, size_t pos) noexcept;
    [[nodiscard]] bool check(std::error_code &ec, size_t TexID, size_t pos) const noexcept;

private:
    GLuint _maxUnits;
    size_t _activePos;
    std::vector<GLuint> _units;
    TextureUnit() noexcept;
};
NAMESPACE_OPENGL_END