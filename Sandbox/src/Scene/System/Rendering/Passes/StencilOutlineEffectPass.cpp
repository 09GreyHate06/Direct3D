#include "StencilOutlineEffectPass.h"
#include "Utils/GlobalAsset.h"
#include "Utils/ShaderCBufs.h"

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
		Renderer::SetTopology(Topology::TriangleList);

		auto basicShader = GlobalAsset::GetShader("basic");
		basicShader->Bind();

		GlobalAsset::GetCBuf("basic_vs_system")->VSBind(basicShader->GetVSResBinding("VSSystemCBuf"));
		GlobalAsset::GetCBuf("basic_vs_entity")->VSBind(basicShader->GetVSResBinding("VSEntityCBuf"));
		GlobalAsset::GetCBuf("basic_ps_entity")->PSBind(basicShader->GetPSResBinding("PSEntityCBuf"));

		GlobalAsset::GetTexture("default")->PSBind(basicShader->GetPSResBinding("tex"));

		cbufs::basic::VSEntityCBuf vsEntityCBuf = {};
		XMMATRIX transformXM = XMLoadFloat4x4(&transform);
		XMStoreFloat4x4(&vsEntityCBuf.transform, XMMatrixTranspose(transformXM));
		GlobalAsset::GetCBuf("basic_vs_entity")->SetData(&vsEntityCBuf);

		cbufs::basic::PSEntityCBuf psEntityCBuf = {};
		psEntityCBuf.color = outliner.color;
		psEntityCBuf.tiling = { 1.0f, 1.0f };
		GlobalAsset::GetCBuf("basic_ps_entity")->SetData(&psEntityCBuf);

		Renderer::DrawIndexed(mesh.iBuffer->GetCount());
	}

	m_components.clear();
}
