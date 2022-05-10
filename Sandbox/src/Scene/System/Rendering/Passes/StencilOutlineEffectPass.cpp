#include "StencilOutlineEffectPass.h"
#include "Utils/ShaderCBufs.h"
#include "Scene/System/Rendering/ResourceLibrary.h"

using namespace d3dcore;
using namespace DirectX;


void StencilOutlineEffectPass::Add(const std::tuple<DirectX::XMFLOAT4X4, MeshComponent, MeshRendererComponent, OutlineComponent>& components)
{
	m_components.emplace_back(components);
}

void StencilOutlineEffectPass::Execute()
{
	for (const auto& components : m_components)
	{
		const auto& [transform, mesh, renderer, outliner] = components;

		mesh.vBuffer->Bind();
		mesh.iBuffer->Bind();
		Renderer::SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		auto basicShader = ResourceLibrary<Shader>::Get("basic");
		basicShader->Bind();

		ResourceLibrary<ConstantBuffer>::Get("basic_vs_system")->VSBind(basicShader->GetVSResBinding("VSSystemCBuf"));
		ResourceLibrary<ConstantBuffer>::Get("basic_vs_entity")->VSBind(basicShader->GetVSResBinding("VSEntityCBuf"));
		ResourceLibrary<ConstantBuffer>::Get("basic_ps_entity")->PSBind(basicShader->GetPSResBinding("PSEntityCBuf"));

		ResourceLibrary<Texture2D>::Get("default")->PSBind(basicShader->GetPSResBinding("tex"));

		cbufs::basic::VSEntityCBuf vsEntityCBuf = {};
		XMMATRIX transformXM = XMLoadFloat4x4(&transform);
		XMStoreFloat4x4(&vsEntityCBuf.transform, XMMatrixTranspose(transformXM));
		ResourceLibrary<ConstantBuffer>::Get("basic_vs_entity")->SetData(&vsEntityCBuf);

		cbufs::basic::PSEntityCBuf psEntityCBuf = {};
		psEntityCBuf.color = outliner.color;
		psEntityCBuf.tiling = { 1.0f, 1.0f };
		ResourceLibrary<ConstantBuffer>::Get("basic_ps_entity")->SetData(&psEntityCBuf);

		Renderer::DrawIndexed(mesh.iBuffer->GetCount());
	}

	m_components.clear();
}
