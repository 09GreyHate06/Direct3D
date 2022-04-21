#include "NormalPass.h"
#include "Utils/ShaderCBufs.h"
#include "Utils/GlobalAsset.h"
#include "Utils/ComponentUtils.h"

using namespace d3dcore;
using namespace d3dcore::utils;
using namespace DirectX;


void NormalPass::Add(const std::tuple<DirectX::XMFLOAT4X4, MeshComponent, MeshRendererComponent, MaterialComponent>& components)
{
	m_components.emplace_back(components);
}

void NormalPass::Execute()
{
	D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	Renderer::SetDepthStencilState(dsDesc);

	for (const auto& components : m_components)
	{
		auto& [transform, mesh, renderer, mat] = components;

		mesh.vBuffer->Bind();
		mesh.iBuffer->Bind();
		Renderer::SetTopology(renderer.topology);

		if (renderer.receiveLight)
		{
			BOOL enableNormalMap = FALSE;

			auto lightingShader = GlobalAsset::GetShader("lighting");
			lightingShader->Bind();

			GlobalAsset::GetCBuf("light_vs_system")->VSBind(lightingShader->GetVSResBinding("VSSystemCBuf"));
			GlobalAsset::GetCBuf("light_vs_entity")->VSBind(lightingShader->GetVSResBinding("VSEntityCBuf"));
			GlobalAsset::GetCBuf("light_ps_system")->PSBind(lightingShader->GetPSResBinding("PSSystemCBuf"));
			GlobalAsset::GetCBuf("light_ps_entity")->PSBind(lightingShader->GetPSResBinding("PSEntityCBuf"));

			if (mat.diffuseMap)
				mat.diffuseMap->PSBind(lightingShader->GetPSResBinding("diffuseMap"));
			else
				GlobalAsset::GetTexture("default")->PSBind(lightingShader->GetPSResBinding("diffuseMap"));

			if (mat.specularMap)
				mat.specularMap->PSBind(lightingShader->GetPSResBinding("specularMap"));
			else
				GlobalAsset::GetTexture("default")->PSBind(lightingShader->GetPSResBinding("specularMap"));

			if (mat.normalMap)
			{
				mat.normalMap->PSBind(lightingShader->GetPSResBinding("normalMap"));
				enableNormalMap = TRUE;
			}
			else
				GlobalAsset::GetTexture("default")->PSBind(lightingShader->GetPSResBinding("normalMap"));



			XMMATRIX transformMatrix = XMLoadFloat4x4(&transform);
			cbufs::light::VSEntityCBuf vsEntityCBuf = {};
			XMStoreFloat4x4(&vsEntityCBuf.entity.transformMatrix, XMMatrixTranspose(transformMatrix));
			XMStoreFloat4x4(&vsEntityCBuf.entity.normalMatrix, XMMatrixInverse(nullptr, transformMatrix));
			GlobalAsset::GetCBuf("light_vs_entity")->SetData(&vsEntityCBuf);

			cbufs::light::PSEntityCBuf psEntityCBuf = {};
			psEntityCBuf.material.diffuseCol = mat.diffuseCol;
			psEntityCBuf.material.tiling = mat.tiling;
			psEntityCBuf.material.shininess = mat.shininess;
			psEntityCBuf.material.enableNormalMap = enableNormalMap;
			GlobalAsset::GetCBuf("light_ps_entity")->SetData(&psEntityCBuf);
		}
		else
		{
			auto basicShader = GlobalAsset::GetShader("basic");
			basicShader->Bind();

			GlobalAsset::GetCBuf("basic_vs_system")->VSBind(basicShader->GetVSResBinding("VSSystemCBuf"));
			GlobalAsset::GetCBuf("basic_vs_entity")->VSBind(basicShader->GetVSResBinding("VSEntityCBuf"));
			GlobalAsset::GetCBuf("basic_ps_entity")->PSBind(basicShader->GetPSResBinding("PSEntityCBuf"));

			if (mat.diffuseMap)
				mat.diffuseMap->PSBind(basicShader->GetPSResBinding("tex"));
			else
				GlobalAsset::GetTexture("default")->PSBind(basicShader->GetPSResBinding("tex"));

			cbufs::basic::VSEntityCBuf vsEntityCBuf = {};
			XMMATRIX transformMatrix = XMLoadFloat4x4(&transform);
			XMStoreFloat4x4(&vsEntityCBuf.transform, XMMatrixTranspose(transformMatrix));
			GlobalAsset::GetCBuf("basic_vs_entity")->SetData(&vsEntityCBuf);

			cbufs::basic::PSEntityCBuf psEntityCBuf = {};
			psEntityCBuf.color = mat.diffuseCol;
			psEntityCBuf.tiling = mat.tiling;
			GlobalAsset::GetCBuf("basic_ps_entity")->SetData(&psEntityCBuf);
		}

		Renderer::DrawIndexed(mesh.iBuffer->GetCount());
	}

	m_components.clear();
}