#include "PhongPass.h"
#include "Scene/Components/Components.h"
#include "Utils/ComponentUtils.h"
#include "Utils/ShaderCBufs.h"

using namespace d3dcore;
using namespace DirectX;

PhongPass::PhongPass(d3dcore::Scene* scene)
	: RenderPass(scene)
{
	m_dirLights.connect(m_scene->GetRegistry(), entt::collector.group<REQ_COMP, DirectionalLightComponent>(entt::exclude<>));
	m_pointLights.connect(m_scene->GetRegistry(), entt::collector.group<REQ_COMP, PointLightComponent>(entt::exclude<>));
	m_spotLights.connect(m_scene->GetRegistry(), entt::collector.group<REQ_COMP, SpotLightComponent>(entt::exclude<>));
	m_renderable.connect(m_scene->GetRegistry(), entt::collector.group<RENDERABLE_COMP>(entt::exclude<>));
}

void PhongPass::Execute()
{
	using namespace d3dcore;
	using namespace DirectX;

	SetLights();

	D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	auto& brt = blendDesc.RenderTarget[0];
	brt.BlendEnable = TRUE;
	brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	brt.BlendOp = D3D11_BLEND_OP_ADD;
	brt.SrcBlendAlpha = D3D11_BLEND_ONE;
	brt.DestBlendAlpha = D3D11_BLEND_ONE;
	brt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	brt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	Renderer::SetBlendState(blendDesc);
	Renderer::SetDepthStencilState(CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT()));

	for (const auto& e : m_renderable)
	{
		const auto& [transform, relationship, mesh, renderer, mat] = m_scene->GetRegistry().get<REQ_COMP, MeshComponent, MeshRendererComponent, MaterialComponent>(e);

		mesh.vBuffer->Bind();
		mesh.iBuffer->Bind();
		Renderer::SetTopology(renderer.topology);

		XMMATRIX transformXM = transform.GetTransform() * GetEntityParentsTransform(relationship);
		if (renderer.receiveLight)
		{
			BOOL enableNormalMap = FALSE;

			auto lightingShader = ResourceLibrary<d3dcore::Shader>::Get("lighting");
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

			cbufs::light::VSEntityCBuf vsEntityCBuf = {};
			XMStoreFloat4x4(&vsEntityCBuf.entity.transformMatrix, XMMatrixTranspose(transformXM));
			XMStoreFloat4x4(&vsEntityCBuf.entity.normalMatrix, XMMatrixInverse(nullptr, transformXM));
			ResourceLibrary<ConstantBuffer>::Get("light_vs_entity")->SetData(&vsEntityCBuf);

			cbufs::light::PSEntityCBuf psEntityCBuf = {};
			psEntityCBuf.material.diffuseCol = mat.diffuseCol;
			psEntityCBuf.material.enableNormalMap = enableNormalMap;
			psEntityCBuf.material.shininess = mat.shininess;
			psEntityCBuf.material.tiling = mat.tiling;
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
			XMStoreFloat4x4(&vsEntityCBuf.transform, XMMatrixTranspose(transformXM));
			ResourceLibrary<ConstantBuffer>::Get("basic_vs_entity")->SetData(&vsEntityCBuf);

			cbufs::basic::PSEntityCBuf psEntityCBuf = {};
			psEntityCBuf.color = mat.diffuseCol;
			psEntityCBuf.tiling = mat.tiling;
			ResourceLibrary<ConstantBuffer>::Get("basic_ps_entity")->SetData(&psEntityCBuf);
		}

		Renderer::DrawIndexed(mesh.iBuffer->GetCount());
	}
}

void PhongPass::SetLights()
{
	using namespace DirectX;
	cbufs::light::PSSystemCbuf psSysCBuf = {};
	psSysCBuf.activeDirLight = m_dirLights.size();
	psSysCBuf.activePointLight = m_pointLights.size();
	psSysCBuf.activeSpotLight = m_spotLights.size();

	uint32_t index = 0;
	for (const auto& e : m_dirLights)
	{
		const auto& [transform, relationship, dirLight] = m_scene->GetRegistry().get<REQ_COMP, DirectionalLightComponent>(e);

		XMMATRIX transformXM = transform.GetTransform() * GetEntityParentsTransform(relationship);

		XMVECTOR directionXM = XMVector4Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), transformXM);
		XMFLOAT3 direction;
		XMStoreFloat3(&direction, directionXM);
		psSysCBuf.dirLights[index].direction = direction;
		psSysCBuf.dirLights[index].ambient = dirLight.color * dirLight.ambientIntensity;
		psSysCBuf.dirLights[index].diffuse = dirLight.color * dirLight.diffuseIntensity;
		psSysCBuf.dirLights[index].specular = dirLight.color * dirLight.specularIntensity;

		index++;
	}

	index = 0;
	for (const auto& e : m_pointLights)
	{
		const auto& [transform, relationship, pointLight] = m_scene->GetRegistry().get<REQ_COMP, PointLightComponent>(e);

		XMMATRIX transformXM = transform.GetTransform() * GetEntityParentsTransform(relationship);

		XMVECTOR positionXM = XMVector4Transform(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), transformXM);
		XMFLOAT3 position;
		XMStoreFloat3(&position, positionXM);
		psSysCBuf.pointLights[index].position = position;
		psSysCBuf.pointLights[index].ambient = pointLight.color * pointLight.ambientIntensity;
		psSysCBuf.pointLights[index].diffuse = pointLight.color * pointLight.diffuseIntensity;
		psSysCBuf.pointLights[index].specular = pointLight.color * pointLight.specularIntensity;
		psSysCBuf.pointLights[index].constant = pointLight.constant;
		psSysCBuf.pointLights[index].linear = pointLight.linear;
		psSysCBuf.pointLights[index].quadratic = pointLight.quadratic;

		index++;
	}

	index = 0;
	for (const auto& e : m_spotLights)
	{
		const auto& [transform, relationship, spotLight] = m_scene->GetRegistry().get<REQ_COMP, SpotLightComponent>(e);

		XMMATRIX transformXM = transform.GetTransform() * GetEntityParentsTransform(relationship);

		XMVECTOR positionXM = XMVector4Transform(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), transformXM);
		XMVECTOR directionXM = XMVector4Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), transformXM);
		XMFLOAT3 position;
		XMFLOAT3 direction;
		XMStoreFloat3(&position, positionXM);
		XMStoreFloat3(&direction, directionXM);

		psSysCBuf.spotLights[index].position = position;
		psSysCBuf.spotLights[index].direction = direction;
		psSysCBuf.spotLights[index].ambient = spotLight.color * spotLight.ambientIntensity;
		psSysCBuf.spotLights[index].diffuse = spotLight.color * spotLight.diffuseIntensity;
		psSysCBuf.spotLights[index].specular = spotLight.color * spotLight.specularIntensity;
		psSysCBuf.spotLights[index].constant = spotLight.constant;
		psSysCBuf.spotLights[index].linear = spotLight.linear;
		psSysCBuf.spotLights[index].quadratic = spotLight.quadratic;
		psSysCBuf.spotLights[index].innerCutOffAngle = cosf(XMConvertToRadians(spotLight.innerCutOffAngle));
		psSysCBuf.spotLights[index].outerCutOffAngle = cosf(XMConvertToRadians(spotLight.outerCutOffAngle));

		index++;
	}

	ResourceLibrary<d3dcore::ConstantBuffer>::Get("light_ps_system")->SetData(&psSysCBuf);
}
