#pragma once
#include "RenderingSystem.h"
#include "Scene/Components/Components.h"
#include "Utils/ShaderCBufs.h"

#include <stack>

using namespace d3dcore;
using namespace d3dcore::utils;
using namespace DirectX;

static XMFLOAT3 operator* (const XMFLOAT3& v, const float s)
{
	return { v.x * s, v.y * s, v.z * s };
}

static XMFLOAT3 operator* (const float s, const XMFLOAT3& v)
{
	return { v.x * s, v.y * s, v.z * s };
}

RenderingSystem::RenderingSystem(d3dcore::Scene* scene)
	: System(scene)
{
	D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	Renderer::SetDepthStencilState(dsDesc);

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

	m_textureShader = Shader::Create("res/shaders/texture.vs.hlsl", "res/shaders/texture.ps.hlsl");
	m_lightShader = Shader::Create("res/shaders/light.vs.hlsl", "res/shaders/light.ps.hlsl");

	Texture2DDesc defTexDesc = {};
	defTexDesc.width = 1;
	defTexDesc.height = 1;
	
	uint32_t white = 0xffffffff;
	m_defaultTexture = Texture2D::Create(&white, defTexDesc);

	m_lightVSSysCBuf = ConstantBuffer::Create(sizeof(cbufs::light::VSSystemCBuf));
	m_lightVSEntityCBuf = ConstantBuffer::Create(sizeof(cbufs::light::VSEntityCBuf));
	m_lightPSSysCBuf = ConstantBuffer::Create(sizeof(cbufs::light::PSSystemCbuf));
	m_lightPSEntityCBuf = ConstantBuffer::Create(sizeof(cbufs::light::PSEntityCBuf));

	m_textureVSSysCBuf = ConstantBuffer::Create(sizeof(cbufs::texture::VSSystemCBuf));
	m_textureVSEntityCBuf = ConstantBuffer::Create(sizeof(cbufs::texture::VSEntityCBuf));
	m_texturePSEntityCBuf = ConstantBuffer::Create(sizeof(cbufs::texture::PSEntityCBuf));

	auto& window = Application::Get().GetWindow();
	FramebufferDesc fbDesc = {};
	fbDesc.width = window.GetWidth();
	fbDesc.height = window.GetHeight();
	m_framebuffer = Framebuffer::Create(fbDesc);
}

RenderingSystem::RenderingSystem()
	: RenderingSystem(nullptr)
{
}

void RenderingSystem::Render(const d3dcore::utils::Camera& camera)
{
	Renderer::SetFramebuffer(m_framebuffer);
	D3D11_VIEWPORT vpDesc = {};
	vpDesc.Width = static_cast<float>(m_framebuffer->GetDesc().width);
	vpDesc.Height = static_cast<float>(m_framebuffer->GetDesc().height);
	vpDesc.TopLeftX = 0.0f;
	vpDesc.TopLeftY = 0.0f;
	vpDesc.MinDepth = 0.0f;
	vpDesc.MaxDepth = 1.0f;
	Renderer::SetViewport(vpDesc);

	Renderer::ClearBuffer(0.1f, 0.1f, 0.1f, 1.0f);

	D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable = TRUE;
	dsDesc.StencilWriteMask = 0xff;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS; // always pass
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE; // replace the value to the reference value
	Renderer::SetDepthStencilState(dsDesc);
	Render_(camera);

	dsDesc.DepthEnable = FALSE;
	dsDesc.StencilEnable = TRUE;
	dsDesc.StencilWriteMask = 0xff;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP; // keep the value
	Renderer::SetDepthStencilState(dsDesc);
	Render_Outline(camera);

	Renderer::SetFramebuffer(nullptr);
}

void RenderingSystem::Render_(const d3dcore::utils::Camera& camera)
{
	SetLigths();

	{
		auto view = GetSceneRegistry().view<TransformComponent, RelationshipComponent, MeshComponent, MeshRendererComponent, MaterialComponent>();
		for (auto& entity : view)
		{
			auto& [transform, relationship, mesh, renderer, mat] =
				view.get<TransformComponent, RelationshipComponent, MeshComponent, MeshRendererComponent, MaterialComponent>(entity);

			mesh.vBuffer->Bind();
			mesh.iBuffer->Bind();
			Renderer::SetTopology(renderer.topology);

			if (renderer.receiveLight)
			{
				BOOL enableNormalMap = FALSE;

				m_lightShader->Bind();

				m_lightVSSysCBuf->VSBind(m_lightShader->GetVSResBinding("VSSystemCBuf"));
				m_lightVSEntityCBuf->VSBind(m_lightShader->GetVSResBinding("VSEntityCBuf"));
				m_lightPSSysCBuf->PSBind(m_lightShader->GetPSResBinding("PSSystemCBuf"));
				m_lightPSEntityCBuf->PSBind(m_lightShader->GetPSResBinding("PSEntityCBuf"));

				if (mat.diffuseMap)
					mat.diffuseMap->PSBind(m_lightShader->GetPSResBinding("diffuseMap"));
				else
					m_defaultTexture->PSBind(m_lightShader->GetPSResBinding("diffuseMap"));

				if (mat.specularMap)
					mat.specularMap->PSBind(m_lightShader->GetPSResBinding("specularMap"));
				else
					m_defaultTexture->PSBind(m_lightShader->GetPSResBinding("specularMap"));

				if (mat.normalMap)
				{
					mat.normalMap->PSBind(m_lightShader->GetPSResBinding("normalMap"));
					enableNormalMap = TRUE;
				}
				else
					m_defaultTexture->PSBind(m_lightShader->GetPSResBinding("normalMap"));

				cbufs::light::VSSystemCBuf vsSysCBuf = {};
				XMStoreFloat4x4(&vsSysCBuf.camera.view, XMMatrixTranspose(camera.GetViewMatrix()));
				XMStoreFloat4x4(&vsSysCBuf.camera.projection, XMMatrixTranspose(camera.GetProjectionMatrix()));
				vsSysCBuf.camera.viewPos = camera.GetDesc().position;
				m_lightVSSysCBuf->SetData(&vsSysCBuf);

				XMMATRIX transformMatrix = transform.GetTransform() * GetEntityParentsTransform(relationship);
				cbufs::light::VSEntityCBuf vsEntityCBuf = {};
				XMStoreFloat4x4(&vsEntityCBuf.entity.transformMatrix, XMMatrixTranspose(transformMatrix));
				XMStoreFloat4x4(&vsEntityCBuf.entity.normalMatrix, XMMatrixInverse(nullptr, transformMatrix));
				m_lightVSEntityCBuf->SetData(&vsEntityCBuf);

				cbufs::light::PSEntityCBuf psEntityCBuf = {};
				psEntityCBuf.material.diffuseCol = mat.diffuseCol;
				psEntityCBuf.material.tiling = mat.tiling;
				psEntityCBuf.material.shininess = mat.shininess;
				psEntityCBuf.material.enableNormalMap = enableNormalMap;
				m_lightPSEntityCBuf->SetData(&psEntityCBuf);
			}
			else
			{
				m_textureShader->Bind();

				m_textureVSSysCBuf->VSBind(m_textureShader->GetVSResBinding("VSSystemCBuf"));
				m_textureVSEntityCBuf->VSBind(m_textureShader->GetVSResBinding("VSEntityCBuf"));
				m_texturePSEntityCBuf->PSBind(m_textureShader->GetPSResBinding("PSEntityCBuf"));

				if (mat.diffuseMap)
					mat.diffuseMap->PSBind(m_textureShader->GetPSResBinding("tex"));
				else
					m_defaultTexture->PSBind(m_textureShader->GetPSResBinding("tex"));

				cbufs::texture::VSSystemCBuf vsSysCBuf = {};
				XMStoreFloat4x4(&vsSysCBuf.view, XMMatrixTranspose(camera.GetViewMatrix()));
				XMStoreFloat4x4(&vsSysCBuf.projection, XMMatrixTranspose(camera.GetProjectionMatrix()));
				m_textureVSSysCBuf->SetData(&vsSysCBuf);

				cbufs::texture::VSEntityCBuf vsEntityCBuf = {};
				XMStoreFloat4x4(&vsEntityCBuf.transform, XMMatrixTranspose(transform.GetTransform() * GetEntityParentsTransform(relationship)));
				m_textureVSEntityCBuf->SetData(&vsEntityCBuf);

				cbufs::texture::PSEntityCBuf psEntityCBuf = {};
				psEntityCBuf.color = mat.diffuseCol;
				psEntityCBuf.tiling = mat.tiling;
				m_texturePSEntityCBuf->SetData(&psEntityCBuf);
			}

			Renderer::DrawIndexed(mesh.iBuffer->GetCount());
		}
	}
}

void RenderingSystem::Render_Outline(const d3dcore::utils::Camera& camera)
{
	auto view = GetSceneRegistry().view<TransformComponent, RelationshipComponent, MeshComponent, MeshRendererComponent, MaterialComponent>();
	for (auto& entity : view)
	{
		auto& [transform, relationship, mesh, renderer, mat] =
			view.get<TransformComponent, RelationshipComponent, MeshComponent, MeshRendererComponent, MaterialComponent>(entity);

		mesh.vBuffer->Bind();
		mesh.iBuffer->Bind();
		Renderer::SetTopology(renderer.topology);

		m_textureShader->Bind();

		m_textureVSSysCBuf->VSBind(m_textureShader->GetVSResBinding("VSSystemCBuf"));
		m_textureVSEntityCBuf->VSBind(m_textureShader->GetVSResBinding("VSEntityCBuf"));
		m_texturePSEntityCBuf->PSBind(m_textureShader->GetPSResBinding("PSEntityCBuf"));

		m_defaultTexture->PSBind(m_textureShader->GetPSResBinding("tex"));

		cbufs::texture::VSSystemCBuf vsSysCBuf = {};
		XMStoreFloat4x4(&vsSysCBuf.view, XMMatrixTranspose(camera.GetViewMatrix()));
		XMStoreFloat4x4(&vsSysCBuf.projection, XMMatrixTranspose(camera.GetProjectionMatrix()));
		m_textureVSSysCBuf->SetData(&vsSysCBuf);

		cbufs::texture::VSEntityCBuf vsEntityCBuf = {};
		TransformComponent oldTransform = transform;
		transform.scale = transform.scale * 1.1f;
		XMStoreFloat4x4(&vsEntityCBuf.transform, XMMatrixTranspose(transform.GetTransform() * GetEntityParentsTransform(relationship)));
		m_textureVSEntityCBuf->SetData(&vsEntityCBuf);
		transform = oldTransform;

		cbufs::texture::PSEntityCBuf psEntityCBuf = {};
		psEntityCBuf.color = { 1.0f, 0.0f, 1.0f, 1.0f };
		psEntityCBuf.tiling = mat.tiling;
		m_texturePSEntityCBuf->SetData(&psEntityCBuf);

		Renderer::DrawIndexed(mesh.iBuffer->GetCount());
	}
}

void RenderingSystem::SetLigths()
{
	cbufs::light::PSSystemCbuf psSysCBuf = {};
	uint32_t index = 0;
	{
		auto view = GetSceneRegistry().view<TransformComponent, RelationshipComponent, DirectionalLightComponent>();
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
		auto view = GetSceneRegistry().view<TransformComponent, RelationshipComponent, PointLightComponent>();
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
		auto view = GetSceneRegistry().view<TransformComponent, RelationshipComponent, SpotLightComponent>();
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

	m_lightPSSysCBuf->SetData(&psSysCBuf);
}

DirectX::XMMATRIX RenderingSystem::GetEntityParentsTransform(d3dcore::RelationshipComponent& relationship)
{
	XMMATRIX result = XMMatrixIdentity();

	if (Entity current = relationship.parent)
	{
		std::stack<XMMATRIX> parentsTransform;
		while (current)
		{
			parentsTransform.push(current.GetComponent<TransformComponent>().GetTransform());
			current = current.GetComponent<RelationshipComponent>().parent;
		}

		while (!parentsTransform.empty())
		{
			result *= parentsTransform.top();
			parentsTransform.pop();
		}
	}

	return result;
}
