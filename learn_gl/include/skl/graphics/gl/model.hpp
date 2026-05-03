#pragma once

#import "skl/graphics/gl/mesh.hpp"
#import "skl/graphics/model.hpp"
 
NAMESPACE_OPENGL_BEGIN
class Model {
public:
    Model() noexcept = default;
    explicit Model(std::error_code &ec, const std::filesystem::path &path, bool flipUVs = false) noexcept;
    void load(std::error_code &ec, const std::filesystem::path &path, bool flipUVs = false) noexcept;

    ~Model() noexcept;
    SKL_DEFAULT_COPYMOVE(Model);

    void Draw(std::error_code &ec, Shader &shader) noexcept;
    static constexpr uint32_t DEFAULT_MESH_SIZE = 64;

private:
    std::vector<Mesh2D> _meshes;
    std::filesystem::path _path;
    std::unordered_map<std::string, Mesh2D::texture_handle_type> _loadedTex;
    void load(std::error_code &ec, bool flipUVs = false) noexcept;
    void processNode(std::error_code &ec, aiNode *node, const aiScene *scene) noexcept;
    Mesh2D processMesh(std::error_code &ec, aiMesh *mesh, const aiScene *scene) noexcept;
    std::vector<Mesh2D::texture_handle_type> loadMaterialTextures(std::error_code &ec, aiMaterial *mat,
                                                                  aiTextureType type, String_view typeName) noexcept;
    // Release texture handles that are exclusive to this model (avoid cache growth)
    void releaseOwnedTextures(std::error_code &ec) noexcept;
};
NAMESPACE_OPENGL_END
