#include "ModelLoader.h"

using namespace d3dcore;
using namespace DirectX;

std::unordered_map<std::string, std::shared_ptr<d3dcore::Texture2D>> ModelLoader::s_textureMaps;
std::filesystem::path ModelLoader::s_directory;



d3dcore::Entity ModelLoader::LoadModel(const std::string& filename, Scene* appScene)
{
    s_directory = filename;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_JoinIdenticalVertices | aiProcess_MakeLeftHanded | aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_GenNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        D3DC_LOG_WARN("Failed to load model: {0}", filename);
        return Entity();
    }

    s_directory = s_directory.lexically_normal();
    s_directory = s_directory.parent_path();

    Entity entity = ProcessNode(scene->mRootNode, scene, Entity(), appScene);

    s_textureMaps.clear();
    s_directory.clear();

    return entity;
}

Entity ModelLoader::ProcessNode(aiNode* node, const aiScene* scene, d3dcore::Entity parent, Scene* appScene)
{
    Entity nodeEntity = appScene->CreateEntity(node->mName.C_Str(), parent);

    for (uint32_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Entity meshEntity = appScene->CreateEntity(mesh->mName.C_Str(), nodeEntity);
        ProcessMesh(mesh, scene, &meshEntity.AddComponent<MeshComponent>(), &meshEntity.AddComponent<MaterialComponent>());
        auto& mr = meshEntity.AddComponent<MeshRendererComponent>();
        mr.receiveLight = true;
        mr.topology = Topology::TriangleList;
    }
    for (uint32_t i = 0; i < node->mNumChildren; i++)
        ProcessNode(node->mChildren[i], scene, nodeEntity, appScene);

    if (node == scene->mRootNode)
        return nodeEntity;
    else
        return {};
}

void ModelLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene, MeshComponent* meshComponent, MaterialComponent* matComponent)
{
    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT2 uv;
        XMFLOAT3 normal;
    };

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve(mesh->mNumVertices);
    for (uint32_t i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex = {};
        vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

        if (mesh->mTextureCoords[0])
            vertex.uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        else
            vertex.uv = { 0.0f, 0.0f };

        vertices.push_back(vertex);
    }

    indices.reserve(mesh->mNumFaces * 3);
    for (uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        const aiFace& face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    std::shared_ptr<Texture2D> diffuseMap = nullptr;
    std::shared_ptr<Texture2D> specularMap = nullptr;

    XMFLOAT3 ambientCol = { 1.0f, 1.0f, 1.0f };
    XMFLOAT3 diffuseCol = { 1.0f, 1.0f, 1.0f };
    XMFLOAT3 specularCol = { 1.0f, 1.0f, 1.0f };
    float shininess = 32.0f;

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        diffuseMap = LoadTextureMap(mat, aiTextureType_DIFFUSE);
        specularMap = LoadTextureMap(mat, aiTextureType_SPECULAR);

        ambientCol = LoadSolidColorMap(mat, AI_MATKEY_COLOR_AMBIENT);
        diffuseCol = LoadSolidColorMap(mat, AI_MATKEY_COLOR_DIFFUSE);
        specularCol = LoadSolidColorMap(mat, AI_MATKEY_COLOR_SPECULAR);


        if (aiGetMaterialFloat(mat, AI_MATKEY_SHININESS, &shininess) != AI_SUCCESS)
            shininess = 32.0f;
    }

    if (!specularMap.get() && diffuseMap.get())
        specularMap = diffuseMap;

    matComponent->diffuseMap = diffuseMap;
    matComponent->specularMap = specularMap;
    matComponent->ambientCol = ambientCol;
    matComponent->diffuseCol = diffuseCol;
    matComponent->specularCol = specularCol;
    matComponent->shininess = shininess;

    VertexBufferDesc vbDesc = {};
    vbDesc.size = static_cast<uint32_t>(vertices.size() * sizeof(Vertex));
    vbDesc.stride = sizeof(Vertex);
    vbDesc.data = vertices.data();
    vbDesc.cpuAccessFlag = 0;
    vbDesc.usage = D3D11_USAGE_DEFAULT;
    std::shared_ptr<VertexBuffer> vb = VertexBuffer::Create(vbDesc);

    IndexBufferDesc ibDesc = {};
    ibDesc.count = indices.size();
    ibDesc.data = indices.data();
    ibDesc.usage = D3D11_USAGE_DEFAULT;
    ibDesc.cpuAccessFlag = 0;
    std::shared_ptr<IndexBuffer> ib = IndexBuffer::Create(ibDesc);

    meshComponent->vBuffer = vb;
    meshComponent->iBuffer = ib;
}

std::shared_ptr<d3dcore::Texture2D> ModelLoader::LoadTextureMap(aiMaterial* mat, aiTextureType type)
{
    if (mat->GetTextureCount(type) <= 0) return nullptr;

    aiString pathBuffer;
    mat->GetTexture(type, 0, &pathBuffer);
    std::string file = (s_directory / pathBuffer.C_Str()).lexically_normal().string();

    if (s_textureMaps.find(file) != s_textureMaps.end())
        return s_textureMaps[file];

    std::shared_ptr<Texture2D> texture = Texture2D::Create(file, false, Texture2DDesc());
    s_textureMaps[file] = texture;
    return texture;
}

DirectX::XMFLOAT3 ModelLoader::LoadSolidColorMap(aiMaterial* mat, const char* key, uint32_t type, uint32_t index)
{
    aiColor3D color;
    if (mat->Get(key, type, index, color) == AI_SUCCESS)
        return { color.r, color.g, color.b };
    return { 1.0f, 1.0f, 1.0f };
}