#pragma once
#include "D3DCore.h"

struct MeshComponent
{
	std::shared_ptr<d3dcore::VertexBuffer> vBuffer = nullptr;
	std::shared_ptr<d3dcore::IndexBuffer> iBuffer = nullptr;
};

struct MeshRendererComponent
{
	d3dcore::Topology topology = d3dcore::Topology::TriangleList;
	bool receiveLight = true;
	// bool castShadow = true;
};

struct MaterialComponent
{
	std::shared_ptr<d3dcore::Texture2D> diffuseMap = nullptr;
	std::shared_ptr<d3dcore::Texture2D> specularMap = nullptr;
	std::shared_ptr<d3dcore::Texture2D> normalMap = nullptr;
	DirectX::XMFLOAT4 diffuseCol = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT2 tiling = { 1.0f, 1.0f };
	float shininess = 32.0f;
};

struct DirectionalLightComponent
{
	DirectX::XMFLOAT3 color = { 1.0f, 1.0f, 1.0f };
	float ambientIntensity = 0.2f;
	float diffuseIntensity = 0.8f;
	float specularIntensity = 1.0f;
};

struct PointLightComponent
{
	DirectX::XMFLOAT3 color = { 1.0f, 1.0f, 1.0f };
	float ambientIntensity = 0.2f;
	float diffuseIntensity = 0.8f;
	float specularIntensity = 1.0f;

	float constant = 1.0f;
	float linear = 0.14f;
	float quadratic = 0.0007f;
};

struct SpotLightComponent
{
	DirectX::XMFLOAT3 color = { 1.0f, 1.0f, 1.0f };
	float ambientIntensity = 0.2f;
	float diffuseIntensity = 0.8f;
	float specularIntensity = 1.0f;

	float constant = 1.0f;
	float linear = 0.14f;
	float quadratic = 0.0007f;

	float innerCutOffAngle = 10.0f;
	float outerCutOffAngle = 15.0f;
};

struct MeshOutlinerComponent
{
	float outlineMult = 1.1f;
	DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

// use this instead of loading each mesh n different entities. Why? it's slow! prolly need multi threading (don't know how)
struct ModelComponent
{
	std::vector<MeshComponent> buffers;
	std::vector<MeshRendererComponent> meshRenderer;
	std::vector<MaterialComponent> materials;
};