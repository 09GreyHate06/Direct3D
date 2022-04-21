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
	D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
	dsDesc.DepthEnable = FALSE;
	dsDesc.StencilEnable = TRUE;
	dsDesc.StencilReadMask = 0xff;
	dsDesc.StencilWriteMask = 0;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	Renderer::SetDepthStencilState(dsDesc);

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

		XMFLOAT3 position = utils::ExtractTranslation(transform);
		XMFLOAT3 rotation = utils::ExtractEulerAngles(transform);
		XMFLOAT3 scale = utils::ExtractScale(transform);

		//if (rotation.x < 0.0f)
		//	rotation.x += XMConvertToRadians(360.0f);

		//if (rotation.y < 0.0f)
		//	rotation.y += XMConvertToRadians(360.0f);

		//if (rotation.z < 0.0f)
		//	rotation.z += XMConvertToRadians(360.0f);

		std::cout << scale.x << ", " << scale.y << ", " << scale.z << '\n';
		XMVECTOR quat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
		XMMATRIX scaledTransform =
			XMMatrixScaling(scale.x, scale.y, scale.z) *
			XMMatrixRotationQuaternion(quat) *
			XMMatrixTranslation(position.x, position.y, position.z);

		cbufs::basic::VSEntityCBuf vsEntityCBuf = {};
		
		XMStoreFloat4x4(&vsEntityCBuf.transform, XMMatrixTranspose(scaledTransform));
		GlobalAsset::GetCBuf("basic_vs_entity")->SetData(&vsEntityCBuf);

		cbufs::basic::PSEntityCBuf psEntityCBuf = {};
		psEntityCBuf.color = outliner.color;
		psEntityCBuf.tiling = { 1.0f, 1.0f };
		GlobalAsset::GetCBuf("basic_ps_entity")->SetData(&psEntityCBuf);

		Renderer::DrawIndexed(mesh.iBuffer->GetCount());
	}

	m_components.clear();
}
