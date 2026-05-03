#import "skl/graphics/error_category.hpp"
#import "skl/graphics/gl/model.hpp"
#import "skl/graphics/resource_manager.hpp"
#include <system_error>

NAMESPACE_OPENGL_BEGIN
Model::Model(std::error_code &ec, const std::filesystem::path &path, bool flipUVs) noexcept
    : _path(path) {
    load(ec, flipUVs);
}

Model::~Model() noexcept {
    std::error_code ec;
    releaseOwnedTextures(ec);
}

void Model::load(std::error_code &ec, const std::filesystem::path &path, bool flipUVs) noexcept {
    _path = path;
    load(ec, flipUVs);
}

void Model::load(std::error_code &ec, bool flipUVs) noexcept {
    ec.clear();
    _meshes.clear();
    _loadedTex.clear();
    Assimp::Importer import;
    auto postProcessFlags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace;
    if (flipUVs) postProcessFlags |= aiProcess_FlipUVs;
    const aiScene *scene = import.ReadFile(_path.string(), postProcessFlags);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        ec = make_error_code(errc::model_load_failed);
        SKL_PRINT(stderr, "{}\n", import.GetErrorString());
        return;
    }
    processNode(ec, scene->mRootNode, scene);
}

void Model::processNode(std::error_code &ec, aiNode *node, const aiScene *scene) noexcept {
    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        auto res = processMesh(ec, mesh, scene);
        if (ec) return;
        _meshes.emplace_back(std::move(res));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(ec, node->mChildren[i], scene);
        if (ec) return;
    }
}

Mesh2D Model::processMesh([[maybe_unused]] std::error_code &ec, aiMesh *mesh, const aiScene *scene) noexcept {
    std::vector<Mesh2D::indices_type> indices;
    std::vector<Mesh2D::vertex_type> vertexs;
    std::vector<Mesh2D::texture_handle_type> textures;

    for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
        Mesh2D::vertex_type vtx;
        vtx.position.x = mesh->mVertices[i].x;
        vtx.position.y = mesh->mVertices[i].y;
        vtx.position.z = mesh->mVertices[i].z;
        if (mesh->HasNormals()) {
            vtx.normal.x = mesh->mNormals[i].x;
            vtx.normal.y = mesh->mNormals[i].y;
            vtx.normal.z = mesh->mNormals[i].z;
        }
        if (mesh->HasTextureCoords(0)) {
            vtx.texcoords.x = mesh->mTextureCoords[0][i].x;
            vtx.texcoords.y = mesh->mTextureCoords[0][i].y;
        } else
            vtx.texcoords = glm::vec3(0.0F);
        vertexs.push_back(vtx);
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; ++j) { indices.push_back(face.mIndices[j]); }
    }

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        auto diffuseMaps = loadMaterialTextures(ec, material, aiTextureType_DIFFUSE, "texture_diffuse");
        if (ec) return {vertexs, indices, textures};
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        auto specularMaps = loadMaterialTextures(ec, material, aiTextureType_SPECULAR, "texture_specular");
        if (ec) return {vertexs, indices, textures};
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return {vertexs, indices, textures};
}

std::vector<Mesh2D::texture_handle_type> Model::loadMaterialTextures(std::error_code &ec, aiMaterial *mat,
                                                                     aiTextureType type,
                                                                     String_view typeName) noexcept {
    ec.clear();
    std::vector<Mesh2D::texture_handle_type> textures;
    const auto count = mat->GetTextureCount(type);
    textures.reserve(count);

    for (uint32_t i = 0; i < count; ++i) {
        aiString str;
        if (mat->GetTexture(type, i, &str) != AI_SUCCESS) {
            ec = make_error_code(errc::model_load_failed);
            return textures;
        }

        std::filesystem::path fullpath = _path.parent_path() / str.C_Str();
        const auto key = fullpath.lexically_normal().string();
        auto cached = _loadedTex.find(key);
        if (cached != _loadedTex.end()) {
            textures.push_back(cached->second);
            continue;
        }

        auto h = ::skl::graphics::TextureMgr().acquire();
        auto tex = ::skl::graphics::TextureMgr().get(h);
        if (!tex) {
            ec = make_error_code(errc::texture_creation_failed);
            return textures;
        }

        tex->load(ec, fullpath);
        if (ec) {
            ::skl::graphics::TextureMgr().release(h, ec);
            ec = make_error_code(errc::texture_creation_failed);
            return textures;
        }

        tex->set_texName(typeName)
            .bind()
            .set_config(GL_TEXTURE_WRAP_S, GL_REPEAT)
            .set_config(GL_TEXTURE_WRAP_T, GL_REPEAT)
            .set_config(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
            .set_config(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
            .update()
            .acquire(ec);
        if (ec) {
            ::skl::graphics::TextureMgr().release(h, ec);
            ec = make_error_code(errc::texture_creation_failed);
            return textures;
        }

        _loadedTex.emplace(key, h);
        textures.push_back(h);
    }

    return textures;
}

void Model::Draw(std::error_code &ec, Shader &shader) noexcept {
    for (auto &mesh : _meshes) {
        mesh.Draw(ec, shader);
        if (ec) return;
    }
}

void Model::releaseOwnedTextures(std::error_code &ec) noexcept {
    ec.clear();
    if (_loadedTex.empty()) return;
    auto &mgr = ::skl::graphics::TextureMgr();
    std::vector<std::string> toErase;
    for (auto &p : _loadedTex) {
        const auto &key = p.first;
        const auto &h = p.second;
        // use const-qualified manager to obtain weak_ptr.use_count() without creating a shared_ptr copy
        const auto &cmgr = static_cast<const ::skl::graphics::TextureManager &>(mgr);
        auto wp = cmgr.get(h);
        if (wp.use_count() <= 1) {
            std::error_code inner;
            mgr.release(h, inner);
            if (!inner) toErase.push_back(key);
        }
    }
    for (auto &k : toErase) _loadedTex.erase(k);
}

NAMESPACE_OPENGL_END
