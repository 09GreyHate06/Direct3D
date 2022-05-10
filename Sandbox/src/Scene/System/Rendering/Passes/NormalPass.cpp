#include "NormalPass.h"
#include "Utils/ShaderCBufs.h"
#include "Utils/ComponentUtils.h"
#include "Scene/System/Rendering/ResourceLibrary.h"

using namespace d3dcore;
using namespace d3dcore::utils;
using namespace DirectX;


void NormalPass::Add(const std::tuple<DirectX::XMFLOAT4X4, MeshComponent, MeshRendererComponent, MaterialComponent>& components)
{
	m_components.emplace_back(components);
}

void NormalPass::Execute()
{
	for (const auto& components : m_components)
	{
		auto& [transform, mesh, renderer, mat] = components;

		mesh.vBuffer->Bind();
		mesh.iBuffer->Bind();
		Renderer::SetTopology(renderer.topology);

		if (renderer.receiveLight)
		{
			BOOL enableNormalMap = FALSE;

			auto lightingShader = ResourceLibrary<Shader>::Get("lighting");
			lightingShader->Bind();

			ResourceLibrary<ConstantBuffer>::Get("light_vs_system")->VSBind(lightingShader->GetVSResBinding("VSSystemCBuf"));
			ResourceLibrary<ConstantBuffer>::Get("light_vs_entity")->VSBind(lightingShader->GetVSResBinding("VSEntityCBuf"));
			ResourceLibrary<ConstantBuffer>::Get("light_ps_system")->PSBind(lightingShader->GetPSResBinding("PSSystemCBuf"));
			ResourceLibrary<ConstantBuffer>::Get("light_ps_entity")->PSBind(lightingShader->GetPSResBinding("PSEntityCBuf"));

			if (mat.diffuseMap)
				mat.diffuseMap->PSBind(lightingShader->GetPSResBinding("diffuseMap"));
			else
				ResourceLibrary<Texture2D>::Get("default")->PSBind(lightingShader->GetPSResBinding("diffuseMap"));

			if (mat.specularMap)
				mat.specularMap->PSBind(lightingShader->GetPSResBinding("specularMap"));
			else
				ResourceLibrary<Texture2D>::Get("default")->PSBind(lightingShader->GetPSResBinding("specularMap"));

			if (mat.normalMap)
			{
				mat.normalMap->PSBind(lightingShader->GetPSResBinding("normalMap"));
				enableNormalMap = TRUE;
			}
			else
				ResourceLibrary<Texture2D>::Get("default")->PSBind(lightingShader->GetPSResBinding("normalMap"));



			XMMATRIX transformXM = XMLoadFloat4x4(&transform);
			cbufs::light::VSEntityCBuf vsEntityCBuf = {};
			XMStoreFloat4x4(&vsEntityCBuf.entity.transformMatrix, XMMatrixTranspose(transformXM));
			XMStoreFloat4x4(&vsEntityCBuf.entity.normalMatrix, XMMatrixInverse(nullptr, transformXM));
			ResourceLibrary<ConstantBuffer>::Get("light_vs_entity")->SetData(&vsEntityCBuf);

			cbufs::light::PSEntityCBuf psEntityCBuf = {};
			psEntityCBuf.material.diffuseCol = mat.diffuseCol;
			psEntityCBuf.material.tiling = mat.tiling;
			psEntityCBuf.material.shininess = mat.shininess;
			psEntityCBuf.material.enableNormalMap = enableNormalMap;
			ResourceLibrary<ConstantBuffer>::Get("light_ps_entity")->SetData(&psEntityCBuf);
		}
		else
		{
			auto basicShader = ResourceLibrary<Shader>::Get("basic");
			basicShader->Bind();

			ResourceLibrary<ConstantBuffer>::Get("basic_vs_system")->VSBind(basicShader->GetVSResBinding("VSSystemCBuf"));
			ResourceLibrary<ConstantBuffer>::Get("basic_vs_entity")->VSBind(basicShader->GetVSResBinding("VSEntityCBuf"));
			ResourceLibrary<ConstantBuffer>::Get("basic_ps_entity")->PSBind(basicShader->GetPSResBinding("PSEntityCBuf"));

			if (mat.diffuseMap)
				mat.diffuseMap->PSBind(basicShader->GetPSResBinding("tex"));
			else
				ResourceLibrary<Texture2D>::Get("default")->PSBind(basicShader->GetPSResBinding("tex"));

			cbufs::basic::VSEntityCBuf vsEntityCBuf = {};
			XMMATRIX transformXM = XMLoadFloat4x4(&transform);
			XMStoreFloat4x4(&vsEntityCBuf.transform, XMMatrixTranspose(transformXM));
			ResourceLibrary<ConstantBuffer>::Get("basic_vs_entity")->SetData(&vsEntityCBuf);

			cbufs::basic::PSEntityCBuf psEntityCBuf = {};
			psEntityCBuf.color = mat.diffuseCol;
			psEntityCBuf.tiling = mat.tiling;
			ResourceLibrary<ConstantBuffer>::Get("basic_ps_entity")->SetData(&psEntityCBuf);
		}

		Renderer::DrawIndexed(mesh.iBuffer->GetCount());
	}

	m_components.clear();
}