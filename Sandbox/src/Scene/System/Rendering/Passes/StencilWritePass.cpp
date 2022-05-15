#include "StencilWritePass.h"
#include "Utils/ComponentUtils.h"
#include "Scene/System/Rendering/ResourceLibrary.h"
#include "Scene/Components/Components.h"
#include "Utils/ComponentUtils.h"
#include "Utils/ShaderCBufs.h"

using namespace d3dcore;
using namespace DirectX;

StencilWritePass::StencilWritePass(d3dcore::Scene* scene)
	: RenderPass(scene)
{
	m_entities.connect(m_scene->GetRegistry(), entt::collector.group<RENDERABLE_COMP, OutlineComponent>(entt::exclude<>));
}

void StencilWritePass::Execute()
{
	D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
	dsDesc.DepthEnable = FALSE;
	dsDesc.StencilEnable = TRUE;
	dsDesc.StencilReadMask = 0x0;
	dsDesc.StencilWriteMask = 0xff;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	Renderer::SetDepthStencilState(dsDesc);

	for (const auto& e : m_entities)
	{
		const auto& [transform, relationship, mesh, renderer] = m_scene->GetRegistry().get<REQ_COMP, MeshComponent, MeshRendererComponent>(e);

		mesh.vBuffer->Bind();
		mesh.iBuffer->Bind();
		Renderer::SetTopology(renderer.topology);

		auto nullpsShader = ResourceLibrary<Shader>::Get("nullps");
		nullpsShader->Bind();

		XMMATRIX transformXM = transform.GetTransform() * GetEntityParentsTransform(relationship);
		ResourceLibrary<ConstantBuffer>::Get("basic_vs_system")->VSBind(nullpsShader->GetVSResBinding("VSSystemCBuf"));
		ResourceLibrary<ConstantBuffer>::Get("basic_vs_entity")->VSBind(nullpsShader->GetVSResBinding("VSEntityCBuf"));

		cbufs::basic::VSEntityCBuf vsEntityCBuf = {};
		XMStoreFloat4x4(&vsEntityCBuf.transform, XMMatrixTranspose(transformXM));
		ResourceLibrary<ConstantBuffer>::Get("basic_vs_entity")->SetData(&vsEntityCBuf);

		Renderer::DrawIndexed(mesh.iBuffer->GetCount());
	}
}
