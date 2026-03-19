#include "skl/graphics/error.hpp"
#include "skl/graphics/gl/texture.hpp"

#include <algorithm>

namespace skl::opengl {
using std::string_view;

namespace {
auto find_cfg_by_pname(std::vector<TexConfig> &cfgs, GLenum pname) noexcept {
    return std::ranges::find_if(cfgs, [pname](const TexConfig &cfg) { return cfg.pname == pname; });
}

void replace_cfgs(std::vector<TexConfig> &current, const std::vector<TexConfig> &incoming) {
    current = incoming;
    std::ranges::for_each(current, [](TexConfig &cfg) { cfg.dirty = true; });
}

void merge_cfgs(std::vector<TexConfig> &current, const std::vector<TexConfig> &incoming, bool &dirty) {
    for (const auto &newCfg : incoming) {
        auto it = find_cfg_by_pname(current, newCfg.pname);
        if (it == current.end()) {
            current.push_back(newCfg);
            current.back().dirty = true;
            continue;
        }

        if (it->param != newCfg.param) {
            it->param = newCfg.param;
            it->dirty = true;
            dirty = true;
        }
    }
}

void append_cfgs(std::vector<TexConfig> &current, const std::vector<TexConfig> &incoming) {
    for (const auto &newCfg : incoming) {
        const bool exists = std::ranges::any_of(
            current, [pname = newCfg.pname](const TexConfig &oldCfg) { return oldCfg.pname == pname; });
        if (!exists) {
            current.push_back(newCfg);
            current.back().dirty = true;
        }
    }
}

void update_cfgs(std::vector<TexConfig> &current, const std::vector<TexConfig> &incoming, bool &dirty) {
    for (const auto &newCfg : incoming) {
        auto it = find_cfg_by_pname(current, newCfg.pname);
        if (it != current.end() && it->param != newCfg.param) {
            it->param = newCfg.param;
            it->dirty = true;
            dirty = true;
        }
    }
}

void remove_cfgs(std::vector<TexConfig> &current, const std::vector<TexConfig> &incoming) {
    std::vector<GLenum> pnames;
    pnames.reserve(incoming.size());
    std::ranges::transform(incoming, std::back_inserter(pnames), [](const TexConfig &cfg) { return cfg.pname; });

    std::erase_if(current,
                  [&pnames](const TexConfig &cfg) { return std::ranges::find(pnames, cfg.pname) != pnames.end(); });
}
}   // namespace

GLuint Texture2D::_bind_id = 0;

template<typename T>
struct is_std_array : std::false_type {
    using type = T;
};

template<typename T, size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {
    using type = T;
};
template<typename T>
inline constexpr auto is_std_array_v = is_std_array<T>::value;

void TexParam::set(Tparam tname, const void *dataIn) noexcept {
    type = tname;
    switch (tname) {
        case I1: data.i = *(int *)dataIn; return;
        case F1: data.f = *(float *)dataIn; return;
        case I4: memcpy(data.iv, dataIn, sizeof(GLint) * 4); return;
        case F4: memcpy(data.fv, dataIn, sizeof(GLfloat) * 4); return;
    }
}

texture_t::texture_t(std::string_view texpath, GLenum intformat, GLenum type, GLint mipLevels) noexcept
    : mipLevels(mipLevels)
    , type(type)
    , intformat(intformat)
    , path(texpath) {}


void TexParam::set(GLint value) noexcept {
    set(I1, &value);
}
void TexParam::set(GLfloat value) noexcept {
    set(F1, &value);
}
void TexParam::set(GLint *value) noexcept {
    set(I4, value);
}
void TexParam::set(GLfloat *value) noexcept {
    set(F4, value);
}
void TexParam::set(std::array<GLint, 4> value) noexcept {
    set(I4, value.data());
}
void TexParam::set(std::array<GLfloat, 4> value) noexcept {
    set(F4, value.data());
}

constexpr bool TexParam::operator==(const TexParam &other) const noexcept {
    if (type != other.type) return false;
    switch (type) {
        case I1: return data.i == other.data.i;
        case F1: return data.f == other.data.f;
        case I4: return !memcmp(data.iv, other.data.iv, 4 * sizeof(GLint));
        case F4: return !memcmp(data.fv, other.data.fv, 4 * sizeof(GLfloat));
    }
    return false;
}

constexpr bool TexParam::operator!=(const TexParam &other) const noexcept {
    if (type != other.type) return false;
    switch (type) {
        case I1: return data.i == other.data.i;
        case F1: return data.f == other.data.f;
        case I4: return memcmp(data.iv, other.data.iv, 4 * sizeof(GLint));
        case F4: return memcmp(data.fv, other.data.fv, 4 * sizeof(GLfloat));
    }
    return false;
}

constexpr bool TexConfig::operator==(const TexConfig &other) const noexcept {
    return (pname != other.pname) && (param == other.param);
}
constexpr bool TexConfig::operator!=(const TexConfig &other) const noexcept {
    return !(*this == other);
}

Texture2D::Texture2D() noexcept
    : _dirty(false)
    , _unitPos(SIZE_T_MAX) {
    glGenTextures(1, &_tex.ID);
}
GLint Texture2D::getGLformat(int channels) noexcept {
    switch (channels) {
        case 1:  return GL_RED;
        case 2:  return GL_RG;
        case 3:  return GL_RGB;
        default: return GL_RGBA;
    }
}

Texture2D::Texture2D(const std::filesystem::path &texpath, GLenum intformat, GLenum type, GLint mipLevels) noexcept
    : _tex(texpath.string(), intformat, type, mipLevels)
    , _dirty(false)
    , _unitPos(SIZE_T_MAX) {
    glGenTextures(1, &_tex.ID);
}
Texture2D::~Texture2D() noexcept {
    if (_unitPos != SIZE_T_MAX) {
        std::error_code ec;
        TextureUnit::instance().release(ec, _unitPos);
        glDeleteTextures(1, &_tex.ID);
    }
    if (_bind_id == _tex.ID) _bind_id = 0;
}

void Texture2D::apply(GLenum pname, const TexParam &param) const noexcept {
    // 边框颜色特殊处理
    if (pname == GL_TEXTURE_BORDER_COLOR) {
        if (param.type == TexParam::I4)
            glTexParameterIiv(GL_TEXTURE_2D, pname, param.data.iv);
        else
            glTexParameterfv(GL_TEXTURE_2D, pname, param.data.fv);
    }
    // 标准参数
    else {
        if (param.type == TexParam::I1)
            glTexParameteri(GL_TEXTURE_2D, pname, param.data.i);
        else
            glTexParameterf(GL_TEXTURE_2D, pname, param.data.f);
    }
}
template<typename T>
Texture2D &Texture2D::set_config(GLenum pname, T value) noexcept {
    TexConfig cfg;
    cfg.pname = pname;
    cfg.param.set(value);
    _cfgs.emplace_back(cfg);
    return *this;
}

Texture2D &Texture2D::bind() noexcept {
    if (_bind_id != _tex.ID) {
        glBindTexture(GL_TEXTURE_2D, _tex.ID);
        _bind_id = _tex.ID;
    }
    return *this;
}

Texture2D &Texture2D::load(std::error_code &ec, const std::filesystem::path &tex, GLint intformat, GLenum type,
                           GLenum mipLevels) noexcept {
    ec.clear();

    _tex.path = tex.string();
    _tex.type = type;
    _tex.mipLevels = (GLint)mipLevels;
    if (type == GL_UNSIGNED_BYTE)
        _tex.data = stbi_load(_tex.path.c_str(), &_tex.width, &_tex.height, &_tex.nrChannels, 0);
    else if (type == GL_FLOAT)
        _tex.data = stbi_loadf(_tex.path.c_str(), &_tex.width, &_tex.height, &_tex.nrChannels, 0);

    else {
        ec = make_error_code(graphics_ec::invalid_argument);
        return *this;
    }
    if (!_tex.data) {
        ec = make_error_code(graphics_ec::texture_creation_failed);
        return *this;
    }

    const auto dataFormat = getGLformat(_tex.nrChannels);
    if (intformat == GL_FALSE) {
        switch (dataFormat) {
            case GL_RED: intformat = GL_R8; break;
            case GL_RG:  intformat = GL_RG8; break;
            case GL_RGB: intformat = GL_RGB8; break;
            default:     intformat = GL_RGBA8; break;
        }
    }
    _tex.intformat = intformat;


    bind();
    glTexStorage2D(GL_TEXTURE_2D, _tex.mipLevels, _tex.intformat, _tex.width, _tex.height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _tex.width, _tex.height, dataFormat, _tex.type, _tex.data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(_tex.data);
    _tex.data = nullptr;

    return *this;
}
template Texture2D &Texture2D::set_config<GLint>(GLenum pname, GLint value) noexcept;
template Texture2D &Texture2D::set_config<GLfloat>(GLenum pname, GLfloat value) noexcept;
template Texture2D &Texture2D::set_config<GLint *>(GLenum pname, GLint *value) noexcept;
template Texture2D &Texture2D::set_config<GLfloat *>(GLenum pname, GLfloat *value) noexcept;
template Texture2D &Texture2D::set_config<std::array<GLint, 4>>(GLenum pname, std::array<GLint, 4> value) noexcept;
template Texture2D &Texture2D::set_config<std::array<GLfloat, 4>>(GLenum pname, std::array<GLfloat, 4> value) noexcept;

Texture2D &Texture2D::set_config(UpdateMode mode, const std::vector<TexConfig> &cfgs) noexcept {
    if (cfgs.empty()) return *this;

    switch (mode) {
        case UpdateMode::replace: replace_cfgs(_cfgs, cfgs); break;
        case UpdateMode::merge:   merge_cfgs(_cfgs, cfgs, _dirty); break;
        case UpdateMode::append:  append_cfgs(_cfgs, cfgs); break;
        case UpdateMode::update:  update_cfgs(_cfgs, cfgs, _dirty); break;
        case UpdateMode::remove:  remove_cfgs(_cfgs, cfgs); break;
    }

    _dirty = true;
    return *this;
}

Texture2D &Texture2D::set_config(UpdateMode policy, std::initializer_list<TexConfig> cfgs) noexcept {
    return set_config(policy, std::vector<TexConfig>(cfgs));
}

Texture2D &Texture2D::update() noexcept {
    if (_cfgs.empty() || !_dirty) return *this;
    bind();
    for (auto &cfg : _cfgs) {
        if (!cfg.dirty) continue;
        apply(cfg);
    }

    _dirty = false;
    return *this;
}

Texture2D &Texture2D::acquire(std::error_code &ec, size_t pos) noexcept {
    ec.clear();
    auto &unitMgr = TextureUnit::instance();
    if (_unitPos == SIZE_T_MAX) {
        auto unitPos = unitMgr.acquire(ec, _tex.ID, pos);
        if (ec) return *this;
        _unitPos = unitPos;
    }
    return *this;
}
Texture2D &Texture2D::activate(std::error_code &ec) noexcept {
    ec.clear();
    auto &unitMgr = TextureUnit::instance();

    unitMgr.activate(ec, _unitPos);
    if (ec) return *this;

    glBindTexture(GL_TEXTURE_2D, _tex.ID);
    return *this;
}

TextureUnit::TextureUnit() noexcept
    : _activePos(-1) {
    GLint maxUnits = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits);
    _maxUnits = maxUnits;
    _units = std::vector(_maxUnits, 0U);
}

TextureUnit &TextureUnit::instance() noexcept {
    static TextureUnit instance;
    return instance;
}

size_t TextureUnit::acquire(std::error_code &ec, GLuint TexID, size_t pos) noexcept {
    ec.clear();

    if (pos != SIZE_T_MAX) {
        if (pos >= _maxUnits)
            ec = make_error_code(graphics_ec::invalid_argument);
        else if (_units[pos] != TexID || _units[pos] != 0)
            ec = make_error_code(graphics_ec::texture_unit_mismatch);
        else
            _units[pos] = TexID;
        return pos;
    }

    auto it = std::ranges::find(_units, 0);
    if (it == _units.end()) {
        ec = make_error_code(graphics_ec::texture_unit_exhausted);
        return -1;
    }
    *it = TexID;

    return std::distance(_units.begin(), it);
}

// 释放单元
void TextureUnit::release(std::error_code &ec, size_t pos) noexcept {
    ec.clear();
    if (pos >= _maxUnits) {
        ec = make_error_code(graphics_ec::invalid_argument);
        return;
    }

    _units[pos] = 0;
    if (_activePos == pos) _activePos = SIZE_T_MAX;
}

void TextureUnit::activate(std::error_code &ec, size_t pos) noexcept {
    ec.clear();

    if (pos >= _maxUnits) {
        ec = make_error_code(graphics_ec::invalid_argument);
        return;
    }

    if (_activePos == pos || _units[pos]) return;

    _activePos = pos;
    glActiveTexture(GL_TEXTURE0 + _activePos);
}


bool TextureUnit::check(std::error_code &ec, size_t TexID, size_t pos) const noexcept {
    ec.clear();
    if (pos >= _maxUnits) {
        ec = make_error_code(graphics_ec::invalid_argument);
        return false;
    }
    return _units[pos] == TexID;
}

}   // namespace skl::opengl
