#pragma once

#include <glad/glad.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#import "skl/graphics/gl/shader.hpp"
#import "skl/graphics/gl/texture.hpp"
#import "skl/graphics/resource_manager.hpp"

NAMESPACE_OPENGL_BEGIN
// constexpr uint32_t MAX_BONE_INFLUENCE = 4;
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 texcoords;
    // glm::vec3 tangent;
    // glm::vec3 bitangent;
    // std::array<int, MAX_BONE_INFLUENCE> boneIDs;
    // std::array<float, MAX_BONE_INFLUENCE> weights;
};

struct Primitive {
    uint32_t VAO;
    uint32_t VBO;
    uint32_t EBO;
};

class Mesh2D {
private:
    struct impl {
        std::vector<Vertex> _vertices;
        std::vector<uint32_t> _indices;
        std::vector<::skl::graphics::TextureHandle> _textures;
        // std::vector<Primitive> _primitives;
        Primitive _primitives;
    };
    std::shared_ptr<impl> _res;

public:
    using vertex_type = decltype(_res->_vertices)::value_type;
    using indices_type = decltype(_res->_indices)::value_type;
    using raw_texture_type = Texture2D;
    using texture_handle_type = decltype(_res->_textures)::value_type;
    using resource_type = decltype(_res)::element_type;
    using shader_type = Shader;

    static constexpr size_t VERTEX_STRIDE = sizeof(vertex_type);
    static constexpr size_t INDICES_STRIDE = sizeof(indices_type);

    Mesh2D(std::vector<vertex_type> &vertices, std::vector<indices_type> &indices,
        std::vector<raw_texture_type> &textures) noexcept;
    Mesh2D(std::vector<vertex_type> &vertices, std::vector<indices_type> &indices,
        std::vector<texture_handle_type> &textures) noexcept;
    ~Mesh2D() noexcept;

    SKL_DEFAULT_COPYABLE(Mesh2D);
    Mesh2D(Mesh2D &&) noexcept;
    Mesh2D &operator=(Mesh2D &&) noexcept;

    Mesh2D &setTex(std::error_code &ec, raw_texture_type &tex, UpdateMode policy = UpdateMode::append) noexcept;
    Mesh2D &setTex(std::error_code &ec, std::vector<raw_texture_type> &tex,
                   UpdateMode policy = UpdateMode::append) noexcept;
    Mesh2D &setTexHandle(std::error_code &ec, const texture_handle_type &h, UpdateMode policy = UpdateMode::append) noexcept;
    Mesh2D &setTexHandle(std::error_code &ec, const std::vector<texture_handle_type> &h,
                   UpdateMode policy = UpdateMode::append) noexcept;
    void Draw(std::error_code &ec, shader_type &shader) noexcept;

private:
    void init() noexcept;
};

NAMESPACE_OPENGL_END
