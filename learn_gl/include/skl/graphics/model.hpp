#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>

#include "skl/graphics/base.hpp"

NAMESPACE_GRAPHICS_BEGIN
class IModel : public IAttribute{
public:
    IModel() noexcept = default;
    ~IModel() noexcept override = default;
    SKL_DEFAULT_COPYMOVE(IModel);
    virtual void Draw(void *shader) noexcept;

protected:
    using mesh_type = void *;
    using texture_type = void *;
    virtual void load(std::error_code&ec, const std::filesystem::path &path) = 0;
    virtual void processNode(aiNode *node, const aiScene *scene) = 0;
    virtual mesh_type processMesh(aiMesh *mesh, const aiScene *scene) = 0;
    virtual std::vector<texture_type> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                                           std::string typeName) = 0;
};

class BaseModel : public IModel {
protected:
std::vector<float> _vertices;
std::vector<uint32_t> _indices;
public:
    void load(std::error_code& ec, const std::filesystem::path &path) noexcept override{
        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            ec = std::make_error_code(std::errc::operation_not_permitted);
        }
    }
};
NAMESPACE_GRAPHICS_END
