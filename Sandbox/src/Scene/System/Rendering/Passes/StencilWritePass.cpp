#include "StencilWritePass.h"
#include "Utils/ComponentUtils.h"
#include "Scene/System/Rendering/ResourceLibrary.h"
#include "Utils/ShaderCBufs.h"

using namespace d3dcore;
using namespace DirectX;

void StencilWritePass::Add(const std::tuple<DirectX::XMFLOAT4X4, MeshComponent, MeshRendererComponent>& components)
{
	m_components.emplace_back(components);
}

void StencilWritePass::Execute()
{
	for (const auto& components : m_components)
	{
		auto& [transform, mesh, renderer] = components;

		mesh.vBuffer->Bind();
		mesh.iBuffer->Bind();
		Renderer::SetTopology(renderer.topology);

		auto shader = ResourceLibrary<Shader>::Get("nullps");
		shader->Bind();

		ResourceLibrary<ConstantBuffer>::Get("basic_vs_system")->VSBind(shader->GetVSResBinding("VSSystemCBuf"));
		ResourceLibrary<ConstantBuffer>::Get("basic_vs_entity")->VSBind(shader->GetVSResBinding("VSEntityCBuf"));

		cbufs::basic::VSEntityCBuf vsEntityCBuf = {};
		XMMATRIX transformMatrix = XMLoadFloat4x4(&transform);
		XMStoreFloat4x4(&vsEntityCBuf.transform, XMMatrixTranspose(transformMatrix));
		ResourceLibrary<ConstantBuffer>::Get("basic_vs_entity")->SetData(&vsEntityCBuf);

		Renderer::DrawIndexed(mesh.iBuffer->GetCount());
	}

	m_components.clear();
}
