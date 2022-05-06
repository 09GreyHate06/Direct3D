#pragma once
#include "RenderingSystem.h"
#include "Scene/Components/Components.h"
#include "Utils/ShaderCBufs.h"
#include "Utils/GlobalAsset.h"
#include "Utils/ComponentUtils.h"
#include "D3DCore/Utils/D3DCMath.h"

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
	brt.SrcBlendAlpha = D3D11_BLEND_ONE;
	brt.DestBlendAlpha = D3D11_BLEND_ONE;
	brt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	brt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	Renderer::SetBlendState(blendDesc);
	Renderer::SetDepthStencilState(DepthStencilMode::Default);

	auto& window = Application::Get().GetWindow();
	RenderTargetDesc rtDesc = {};
	rtDesc.width = window.GetWidth();
	rtDesc.height = window.GetHeight();
	rtDesc.samples = 4;
	rtDesc.sampleQuality = 1;
	m_msRenderTarget = RenderTarget::Create(rtDesc);

	rtDesc.samples = 1;
	rtDesc.sampleQuality = 0;
	m_renderTarget = RenderTarget::Create(rtDesc);
	SamplerState sampler = {};
	sampler.filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampler.addressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler.addressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	m_renderTarget->SetTexture2DSamplerState(sampler);

	DepthStencilDesc dsDesc = {};
	dsDesc.width = window.GetWidth();
	dsDesc.height = window.GetHeight();
	dsDesc.samples = 4;
	dsDesc.sampleQuality = 1;
	m_msDepthStencil = DepthStencil::Create(dsDesc);
	//SetBlurCBuf();
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
		}
	}

	Render_();
	RenderTarget::Resolve(m_renderTarget.get(), m_msRenderTarget.get());
}

void RenderingSystem::SetRenderTargetSize(uint32_t width, uint32_t height)
{
	m_msRenderTarget->Resize(width, height);
	m_renderTarget->Resize(width, height);
	m_msDepthStencil->Resize(width, height);
}

void RenderingSystem::Render_()
{
	D3D11_VIEWPORT vp = {};
	vp.Width = (float)m_msRenderTarget->GetDesc().width;
	vp.Height = (float)m_msRenderTarget->GetDesc().height;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	Renderer::SetViewport(vp);

	Renderer::SetRenderTarget(m_msRenderTarget.get());
	Renderer::SetDepthStencil(m_msDepthStencil.get());
	Renderer::ClearActiveRTV(0.0f, 0.0f, 0.0f, 0.0f);
	Renderer::ClearActiveDSV(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_normalPass.Execute();

	Renderer::SetRenderTarget(DEFAULT_RENDER_TARGET);
	Renderer::SetDepthStencil(nullptr);
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