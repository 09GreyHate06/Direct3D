#pragma once

#include "D3DCore.h"
#include "Scene/Components/Components.h"
#include <queue>

class StencilWritePass
{
public:
	StencilWritePass() = default;
	void Add(const std::tuple<DirectX::XMFLOAT4X4, MeshComponent, MeshRendererComponent>& components);

	void Execute();

private:
	std::vector<std::tuple<DirectX::XMFLOAT4X4, MeshComponent, MeshRendererComponent>> m_components;
};