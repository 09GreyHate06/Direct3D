#pragma once

#include "D3DCore.h"
#include "Scene/Components/Components.h"
#include <filesystem>

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT3 bitangent;
};

class ModelLoader
{
public:
	static d3dcore::Entity LoadModel(const std::string& filename, d3dcore::Scene* appScene);

private:
	static d3dcore::Entity ProcessNode(aiNode* node, const aiScene* scene, d3dcore::Entity parent, d3dcore::Scene* appScene);
	static void ProcessMesh(aiMesh* mesh, const aiScene* scene, MeshComponent* meshComponent, MaterialComponent* matComponent);
	static std::shared_ptr<d3dcore::Texture2D> LoadTextureMap(aiMaterial* mat, aiTextureType type);
	static DirectX::XMFLOAT3 LoadSolidColorMap(aiMaterial* mat, const char* key, uint32_t type, uint32_t index);

private:
	static std::unordered_map<std::string, std::shared_ptr<d3dcore::Texture2D>> s_textureMaps;
	static std::filesystem::path s_directory;
};