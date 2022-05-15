#include "StencilOutlineEffectPass.h"
#include "Utils/ShaderCBufs.h"
#include "Scene/System/Rendering/ResourceLibrary.h"
#include "Scene/Components/Components.h"
#include "Utils/ComponentUtils.h"

using namespace d3dcore;
using namespace DirectX;

StencilOutlineEffectPass::StencilOutlineEffectPass(d3dcore::Scene* scene)
	: RenderPass(scene)
{
	m_entities.connect(m_scene->GetRegistry(), entt::collector.group<RENDERABLE_COMP, OutlineComponent>(entt::exclude<>));
}

void StencilOutlineEffectPass::Execute()
{
	Renderer::SetDepthStencilState(CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT()));
	Renderer::SetBlendState(CD3D11_BLEND_DESC(CD3D11_DEFAULT()));

	for (const auto& e : m_entities)
	{
		const auto& [transform, relationship, mesh, renderer, outline] = m_scene->GetRegistry().get<REQ_COMP, MeshComponent, MeshRendererComponent, OutlineComponent>(e);

		mesh.vBuffer->Bind();
		mesh.iBuffer->Bind();
		Renderer::SetTopology(renderer.topology);

		auto basicShader = ResourceLibrary<Shader>::Get("basic");
		basicShader->Bind();

		ResourceLibrary<ConstantBuffer>::Get("basic_vs_system")->VSBind(basicShader->GetVSResBinding("VSSystemCBuf"));
		ResourceLibrary<ConstantBuffer>::Get("basic_vs_entity")->VSBind(basicShader->GetVSResBinding("VSEntityCBuf"));
		ResourceLibrary<ConstantBuffer>::Get("basic_ps_entity")->PSBind(basicShader->GetPSResBinding("PSEntityCBuf"));

		ResourceLibrary<Texture2D>::Get("default")->PSBind(basicShader->GetPSResBinding("tex"));

		cbufs::basic::VSEntityCBuf vsEntityCBuf = {};
		XMMATRIX transformXM = transform.GetTransform() * GetEntityParentsTransform(relationship);
		XMStoreFloat4x4(&vsEntityCBuf.transform, XMMatrixTranspose(transformXM));
		ResourceLibrary<ConstantBuffer>::Get("basic_vs_entity")->SetData(&vsEntityCBuf);

		cbufs::basic::PSEntityCBuf psEntityCBuf = {};
		psEntityCBuf.color = outline.color;
		psEntityCBuf.tiling = { 1.0f, 1.0f };
		ResourceLibrary<ConstantBuffer>::Get("basic_ps_entity")->SetData(&psEntityCBuf);

		Renderer::DrawIndexed(mesh.iBuffer->GetCount());
	}
}
