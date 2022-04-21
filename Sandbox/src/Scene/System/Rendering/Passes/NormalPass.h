#pragma once
#include "D3DCore.h"
#include "Scene/Components/Components.h"
#include <queue>

class NormalPass
{
public:
	NormalPass() = default;

	void Add(const std::tuple<DirectX::XMFLOAT4X4, 
		MeshComponent, MeshRendererComponent, MaterialComponent>& components);
	void Execute();

private:
	std::vector<std::tuple<DirectX::XMFLOAT4X4,
		MeshComponent, MeshRendererComponent, MaterialComponent>> m_components;
};