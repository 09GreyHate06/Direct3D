#pragma once
#include "D3DCore.h"
#include "Scene/Components/Components.h"

class StencilOutlineEffectPass
{
public:
	StencilOutlineEffectPass() = default;

	void Add(const std::tuple<DirectX::XMFLOAT4X4,
		MeshComponent, MeshRendererComponent, OutlineComponent>& components);

	void Execute();
private:
	std::vector<std::tuple<DirectX::XMFLOAT4X4,
		MeshComponent, MeshRendererComponent, OutlineComponent>> m_components;
};