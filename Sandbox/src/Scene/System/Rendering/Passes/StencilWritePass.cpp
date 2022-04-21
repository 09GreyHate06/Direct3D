#include "StencilWritePass.h"
#include "Utils/ComponentUtils.h"
#include "Utils/GlobalAsset.h"
#include "Utils/ShaderCBufs.h"

using namespace d3dcore;
using namespace DirectX;

void StencilWritePass::Add(const std::tuple<DirectX::XMFLOAT4X4, MeshComponent, MeshRendererComponent>& components)
{
	m_components.emplace_back(components);
}

void StencilWritePass::Execute()
{
	D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.StencilEnable = TRUE;
	dsDesc.StencilWriteMask = 0xff;
	dsDesc.StencilReadMask = 0;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	Renderer::SetDepthStencilState(dsDesc);

	for (const auto& components : m_components)
	{
		auto& [transform, mesh, renderer] = components;

		mesh.vBuffer->Bind();
		mesh.iBuffer->Bind();
		Renderer::SetTopology(renderer.topology);

		auto shader = GlobalAsset::GetShader("nullps");
		shader->Bind();

		GlobalAsset::GetCBuf("basic_vs_system")->VSBind(shader->GetVSResBinding("VSSystemCBuf"));
		GlobalAsset::GetCBuf("basic_vs_entity")->VSBind(shader->GetVSResBinding("VSEntityCBuf"));

		cbufs::basic::VSEntityCBuf vsEntityCBuf = {};
		XMMATRIX transformMatrix = XMLoadFloat4x4(&transform);
		XMStoreFloat4x4(&vsEntityCBuf.transform, XMMatrixTranspose(transformMatrix));
		GlobalAsset::GetCBuf("basic_vs_entity")->SetData(&vsEntityCBuf);

		Renderer::DrawIndexed(mesh.iBuffer->GetCount());
	}

	m_components.clear();
}
