#pragma once
#include "RenderingSystem.h"
#include "Scene/Components/Components.h"
#include "Utils/ShaderCBufs.h"
#include "Utils/GlobalAsset.h"
#include "Utils/ComponentUtils.h"

#include <stack>

using namespace d3dcore;
using namespace d3dcore::utils;
using namespace DirectX;

RenderingSystem::RenderingSystem(d3dcore::Scene* scene)
	: m_scene(scene)
{
	D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	auto& brt = blendDesc.RenderTarget[0];
	brt.BlendEnable = TRUE;
	brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	brt.BlendOp = D3D11_BLEND_OP_ADD;
	brt.SrcBlendAlpha = D3D11_BLEND_ZERO;
	brt.DestBlendAlpha = D3D11_BLEND_ONE;
	brt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	brt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	Renderer::SetBlendState(blendDesc);

	GlobalAsset::LoadAndAddShader("basic", "res/shaders/basic.vs.hlsl", "res/shaders/basic.ps.hlsl");
	GlobalAsset::LoadAndAddShader("lighting", "res/shaders/light.vs.hlsl", "res/shaders/light.ps.hlsl");
	GlobalAsset::LoadAndAddShader("nullps", "res/shaders/basic.vs.hlsl");

	Texture2DDesc defTexDesc = {};
	defTexDesc.width = 1;
	defTexDesc.height = 1;

	uint32_t white = 0xffffffff;
	GlobalAsset::LoadAndAddTexture("default", &white, defTexDesc);

	//std::cout << sizeof(cbufs::light::VSSystemCBuf) << '\n';
	//std::cout << sizeof(cbufs::light::VSEntityCBuf)	<< '\n';
	//std::cout << sizeof(cbufs::light::PSSystemCbuf)	<< '\n';
	//std::cout << sizeof(cbufs::light::PSEntityCBuf)	<< '\n';

	//std::cout << sizeof(cbufs::basic::VSSystemCBuf) << '\n';
	//std::cout << sizeof(cbufs::basic::VSEntityCBuf) << '\n';
	//std::cout << sizeof(cbufs::basic::PSEntityCBuf) << '\n';


	GlobalAsset::LoadAndAddCBuf("light_vs_system", sizeof(cbufs::light::VSSystemCBuf));
	GlobalAsset::LoadAndAddCBuf("light_vs_entity", sizeof(cbufs::light::VSEntityCBuf));
	GlobalAsset::LoadAndAddCBuf("light_ps_system", sizeof(cbufs::light::PSSystemCbuf));
	GlobalAsset::LoadAndAddCBuf("light_ps_entity", sizeof(cbufs::light::PSEntityCBuf));
	
	GlobalAsset::LoadAndAddCBuf("basic_vs_system", sizeof(cbufs::basic::VSSystemCBuf));
	GlobalAsset::LoadAndAddCBuf("basic_vs_entity", sizeof(cbufs::basic::VSEntityCBuf));
	GlobalAsset::LoadAndAddCBuf("basic_ps_entity", sizeof(cbufs::basic::PSEntityCBuf));

	auto& window = Application::Get().GetWindow();
	FramebufferDesc fbDesc = {};
	fbDesc.width = window.GetWidth();
	fbDesc.height = window.GetHeight();
	fbDesc.samples = 4;
	fbDesc.sampleQuality = 1;
	fbDesc.hasDepth = true;
	m_msFramebuffer = Framebuffer::Create(fbDesc);

	fbDesc.samples = 1;
	fbDesc.sampleQuality = 0;
	fbDesc.hasDepth = true;
	m_framebuffer = Framebuffer::Create(fbDesc);
}

RenderingSystem::RenderingSystem()
	: RenderingSystem(nullptr)
{
}

void RenderingSystem::SetScene(d3dcore::Scene* scene)
{
	m_scene = scene;
}

void RenderingSystem::Render(const d3dcore::utils::Camera& camera)
{
	Renderer::SetFramebuffer(m_msFramebuffer);
	D3D11_VIEWPORT vpDesc = {};
	vpDesc.Width = static_cast<float>(m_msFramebuffer->GetDesc().width);
	vpDesc.Height = static_cast<float>(m_msFramebuffer->GetDesc().height);
	vpDesc.TopLeftX = 0.0f;
	vpDesc.TopLeftY = 0.0f;
	vpDesc.MinDepth = 0.0f;
	vpDesc.MaxDepth = 1.0f;
	Renderer::SetViewport(vpDesc);

	Renderer::ClearBuffer(0.1f, 0.1f, 0.1f, 1.0f);

	SetLigths();

	cbufs::light::VSSystemCBuf vsLightSysCBuf = {};
	XMStoreFloat4x4(&vsLightSysCBuf.camera.view, XMMatrixTranspose(camera.GetViewMatrix()));
	XMStoreFloat4x4(&vsLightSysCBuf.camera.projection, XMMatrixTranspose(camera.GetProjectionMatrix()));
	vsLightSysCBuf.camera.viewPos = camera.GetDesc().position;
	GlobalAsset::GetCBuf("light_vs_system")->SetData(&vsLightSysCBuf);


	cbufs::basic::VSSystemCBuf vsTexSysCBuf = {};
	XMStoreFloat4x4(&vsTexSysCBuf.view, XMMatrixTranspose(camera.GetViewMatrix()));
	XMStoreFloat4x4(&vsTexSysCBuf.projection, XMMatrixTranspose(camera.GetProjectionMatrix()));
	GlobalAsset::GetCBuf("basic_vs_system")->SetData(&vsTexSysCBuf);

	{
		const auto view = m_scene->GetRegistry().view<TransformComponent, RelationshipComponent, MeshComponent, MeshRendererComponent, MaterialComponent>();
		for (auto& entity : view)
		{
			const auto& [transform, relationship, mesh, renderer, mat] =
				view.get<TransformComponent, RelationshipComponent, MeshComponent, MeshRendererComponent, MaterialComponent>(entity);

			XMMATRIX transformMatrixXM = transform.GetTransform() * GetEntityParentsTransform(relationship);
			XMFLOAT4X4 transformMatrix;
			XMStoreFloat4x4(&transformMatrix, transformMatrixXM);

			m_normalPass.Add({ transformMatrix, mesh, renderer, mat });

			if (m_scene->GetRegistry().any_of<OutlineComponent>(entity))
			{
				m_stencilWritePass.Add({ transformMatrix, mesh, renderer });
				m_stencilOutlinePass.Add({ transformMatrix, mesh, renderer, m_scene->GetRegistry().get<OutlineComponent>(entity) });
			}
		}
	}

	Render_();
	Framebuffer::Resolve(m_framebuffer.get(), m_msFramebuffer.get());
	Renderer::SetFramebuffer(nullptr);
}

void RenderingSystem::Render_()
{
	m_normalPass.Execute();
	m_stencilWritePass.Execute();
	m_stencilOutlinePass.Execute();
}

void RenderingSystem::SetLigths()
{
	cbufs::light::PSSystemCbuf psSysCBuf = {};
	uint32_t index = 0;
	{
		auto view = m_scene->GetRegistry().view<TransformComponent, RelationshipComponent, DirectionalLightComponent>();
		for (auto& entity : view)
		{
			auto& [transform, relationship, light] = view.get<TransformComponent, RelationshipComponent, DirectionalLightComponent>(entity);

			XMMATRIX transformMatrix = transform.GetTransform() * GetEntityParentsTransform(relationship);

			XMVECTOR directionVec = XMVector4Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), transformMatrix);
			XMFLOAT3 direction;
			XMStoreFloat3(&direction, directionVec);
			psSysCBuf.dirLights[index].direction = direction;
			psSysCBuf.dirLights[index].ambient = light.color * light.ambientIntensity;
			psSysCBuf.dirLights[index].diffuse = light.color * light.diffuseIntensity;
			psSysCBuf.dirLights[index].specular = light.color * light.specularIntensity;

			index++;
		}

		psSysCBuf.activeDirLight = static_cast<uint32_t>(view.size_hint());
	}

	index = 0;
	{
		auto view = m_scene->GetRegistry().view<TransformComponent, RelationshipComponent, PointLightComponent>();
		for (auto& entity : view)
		{
			auto& [transform, relationship, light] = view.get<TransformComponent, RelationshipComponent, PointLightComponent>(entity);
			XMMATRIX transformMatrix = transform.GetTransform() * GetEntityParentsTransform(relationship);
			XMVECTOR positionVec = XMVector4Transform(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), transformMatrix);
			XMFLOAT3 position;
			XMStoreFloat3(&position, positionVec);
			psSysCBuf.pointLights[index].position = position;
			psSysCBuf.pointLights[index].ambient = light.color * light.ambientIntensity;
			psSysCBuf.pointLights[index].diffuse = light.color * light.diffuseIntensity;
			psSysCBuf.pointLights[index].specular = light.color * light.specularIntensity;
			psSysCBuf.pointLights[index].constant = light.constant;
			psSysCBuf.pointLights[index].linear = light.linear;
			psSysCBuf.pointLights[index].quadratic = light.quadratic;

			index++;
		}

		psSysCBuf.activePointLight = static_cast<uint32_t>(view.size_hint());
	}

	index = 0;
	{
		auto view = m_scene->GetRegistry().view<TransformComponent, RelationshipComponent, SpotLightComponent>();
		for (auto& entity : view)
		{
			auto& [transform, relationship, light] = view.get<TransformComponent, RelationshipComponent, SpotLightComponent>(entity);

			XMMATRIX transformMatrix = transform.GetTransform() * GetEntityParentsTransform(relationship);
			XMVECTOR positionVec = XMVector4Transform(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), transformMatrix);
			XMVECTOR directionVec = XMVector4Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), transformMatrix);
			XMFLOAT3 position;
			XMFLOAT3 direction;
			XMStoreFloat3(&position, positionVec);
			XMStoreFloat3(&direction, directionVec);

			psSysCBuf.spotLights[index].position = position;
			psSysCBuf.spotLights[index].direction = direction;
			psSysCBuf.spotLights[index].ambient = light.color * light.ambientIntensity;
			psSysCBuf.spotLights[index].diffuse = light.color * light.diffuseIntensity;
			psSysCBuf.spotLights[index].specular = light.color * light.specularIntensity;
			psSysCBuf.spotLights[index].constant = light.constant;
			psSysCBuf.spotLights[index].linear = light.linear;
			psSysCBuf.spotLights[index].quadratic = light.quadratic;
			psSysCBuf.spotLights[index].innerCutOffAngle = cosf(XMConvertToRadians(light.innerCutOffAngle));
			psSysCBuf.spotLights[index].outerCutOffAngle = cosf(XMConvertToRadians(light.outerCutOffAngle));

			index++;
		}

		psSysCBuf.activeSpotLight = static_cast<uint32_t>(view.size_hint());
	}

	GlobalAsset::GetCBuf("light_ps_system")->SetData(&psSysCBuf);
}


