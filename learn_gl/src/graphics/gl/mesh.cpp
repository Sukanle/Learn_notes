#include "skl/graphics/error_category.hpp"
#include "skl/graphics/gl/mesh.hpp"
#include "skl/graphics/resource_manager.hpp"

NAMESPACE_OPENGL_BEGIN
namespace {
template<typename T>
concept has_keyfn = requires(T var) { var.getKey(); };
template<typename T>
    requires has_keyfn<T>
auto find_by(std::vector<T> &container, ssize_t key) noexcept {
    return std::ranges::find_if(container, [key](const T &element) { return element.getKey() == key; });
}
}   // namespace

Mesh2D::Mesh2D(std::vector<Mesh2D::vertex_type> &vertices, std::vector<Mesh2D::indices_type> &indices,
               std::vector<Mesh2D::raw_texture_type> &textures) noexcept
    : _res(std::make_shared<resource_type>()) {
    _res->_vertices = std::exchange(vertices, {});
    _res->_indices = std::exchange(indices, {});
    auto local_textures = std::exchange(textures, {});
    for (auto &tex : local_textures) {
        auto h = ::skl::graphics::TextureMgr().acquire([&tex](auto &t) { t = std::move(tex); });
        _res->_textures.push_back(h);
    }
    init();
}

Mesh2D::Mesh2D(std::vector<Mesh2D::vertex_type> &vertices, std::vector<Mesh2D::indices_type> &indices,
               std::vector<Mesh2D::texture_handle_type> &textures) noexcept
    : _res(std::make_shared<resource_type>()) {
    _res->_vertices = std::exchange(vertices, {});
    _res->_indices = std::exchange(indices, {});
    _res->_textures = std::exchange(textures, {});
    init();
}

Mesh2D::Mesh2D(Mesh2D &&other) noexcept
    : _res(std::move(other._res)) {}

Mesh2D &Mesh2D::operator=(Mesh2D &&other) noexcept {
    if (this != &other) {
        _res = std::move(other._res);
    }
    return *this;
}

Mesh2D::~Mesh2D() noexcept {
    if (!_res) return;
    glDeleteVertexArrays(1, &_res->_primitives.VAO);
    glDeleteBuffers(1, &_res->_primitives.VBO);
    glDeleteBuffers(1, &_res->_primitives.EBO);
}

Mesh2D &Mesh2D::setTex(std::error_code &ec, Texture2D &tex, UpdateMode policy) noexcept {
    ec.clear();
    if ((policy != UpdateMode::append) && (policy != UpdateMode::remove)) {
        ec = make_error_code(errc::invalid_argument);
        return *this;
    }
    auto TexName = tex.getTex().name;
    auto h = ::skl::graphics::TextureMgr().acquire([&tex](auto &t) mutable { t = std::move(tex); });
    if (policy == UpdateMode::append)
        _res->_textures.push_back(h);
    else if (policy == UpdateMode::remove) {
        // remove matching handles by comparing ID if possible
        auto it = std::ranges::find_if(_res->_textures, [&](const auto &hh) {
            auto sp = ::skl::graphics::TextureMgr().get(hh);
            return sp && (sp->getTex().name == TexName);
        });
        if (it != _res->_textures.end()) _res->_textures.erase(it);
    }
    return *this;
}

Mesh2D &Mesh2D::setTex(std::error_code &ec, std::vector<Texture2D> &tex, UpdateMode policy) noexcept {
    ec.clear();
    if ((policy != UpdateMode::append) && (policy != UpdateMode::remove)) {
        ec = make_error_code(errc::invalid_argument);
        return *this;
    }
    for (auto &t : tex) {
        setTex(ec, t, policy);
        if (ec) return *this;
    }
    return *this;
}

Mesh2D &Mesh2D::setTexHandle(std::error_code &ec, const texture_handle_type &h, UpdateMode policy) noexcept {
    ec.clear();
    if ((policy != UpdateMode::append) && (policy != UpdateMode::remove)) {
        ec = make_error_code(errc::invalid_argument);
        return *this;
    }
    if (policy == UpdateMode::append)
        _res->_textures.push_back(h);
    else if (policy == UpdateMode::remove) {
        auto it = std::ranges::find(_res->_textures, h);
        if (it != _res->_textures.end()) _res->_textures.erase(it);
    }
    return *this;
}

void Mesh2D::Draw(std::error_code &ec, Mesh2D::shader_type &shader) noexcept {
    ec.clear();
    if (!_res) return;
    uint32_t diffuseNr = 1;
    uint32_t specularNr = 1;
    for (auto &h : _res->_textures) {
        auto sp = ::skl::graphics::TextureMgr().get(h);
        if (!sp) {
            ec = make_error_code(errc::invalid_argument);
            return;
        }
        sp->activate(ec);
        if (ec) return;
        if (sp->getTex().name == "texture_diffuse") {
            shader.set1I(("material.diffuse" + std::to_string(diffuseNr++)).c_str(), sp->getUnitPos());
        } else if (sp->getTex().name == "texture_specular") {
            shader.set1I(("material.specular" + std::to_string(specularNr++)).c_str(), sp->getUnitPos());
        }
    }

    // TODO: Add vector support for Primitive
    // draw mesh
    glBindVertexArray(_res->_primitives.VAO);
    glDrawElements(GL_TRIANGLES, _res->_indices.size(), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    // Set default configure
    glActiveTexture(GL_TEXTURE0);
}
void Mesh2D::init() noexcept {
    glGenVertexArrays(1, &_res->_primitives.VAO);
    glGenBuffers(1, &_res->_primitives.VBO);
    glGenBuffers(1, &_res->_primitives.EBO);

    glBindVertexArray(_res->_primitives.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _res->_primitives.VBO);

    glBufferData(GL_ARRAY_BUFFER, _res->_vertices.size() * VERTEX_STRIDE, _res->_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _res->_primitives.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _res->_indices.size() * INDICES_STRIDE, _res->_indices.data(),
                 GL_STATIC_DRAW);

    // 顶点位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_STRIDE, (void *)0);
    glEnableVertexAttribArray(0);
    // 顶点法线
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VERTEX_STRIDE, (void *)offsetof(vertex_type, normal));
    glEnableVertexAttribArray(1);
    // 顶点纹理坐标
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, VERTEX_STRIDE, (void *)offsetof(vertex_type, texcoords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

NAMESPACE_OPENGL_END
