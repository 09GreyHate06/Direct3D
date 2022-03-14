#pragma once
#include "RenderingSystem.h"
#include "Scene/Components/Components.h"

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

namespace cbufs
{
	static constexpr uint32_t s_lightMaxCount = 32;

	struct LightVSSystemCBuf
	{
		struct
		{
			DirectX::XMFLOAT4X4 view;
			DirectX::XMFLOAT4X4 projection;
			DirectX::XMFLOAT3 viewPos;
			float p0;
		} camera;
	};

	struct LightVSEntityCBuf
	{
		struct 
		{
			DirectX::XMFLOAT4X4 transformMatrix;
			DirectX::XMFLOAT4X4 normalMatrix;
		} entity;
	};

	struct LightPSSystemCbuf
	{
		struct
		{
			DirectX::XMFLOAT3 direction;
			float p0;
			DirectX::XMFLOAT3 ambient;
			float p1;
			DirectX::XMFLOAT3 diffuse;
			float p2;
			DirectX::XMFLOAT3 specular;
			float p3;
		} dirLights[s_lightMaxCount];

		struct 
		{
			DirectX::XMFLOAT3 position;
			float p0;
			DirectX::XMFLOAT3 ambient;
			float constant;
			DirectX::XMFLOAT3 diffuse;
			float linear;
			DirectX::XMFLOAT3 specular;
			float quadratic;
		} pointLights[s_lightMaxCount];

		struct 
		{
			DirectX::XMFLOAT3 position;
			float constant;

			DirectX::XMFLOAT3 direction;
			float linear;

			DirectX::XMFLOAT3 ambient;
			float quadratic;

			DirectX::XMFLOAT3 diffuse;
			float innerCutOffAngle;

			DirectX::XMFLOAT3 specular;
			float outerCutOffAngle;
		} spotLights[s_lightMaxCount];

		uint32_t activeDirLight;
		uint32_t activePointLight;
		uint32_t activeSpotLight;
		uint32_t p0;
	};

	struct LightPSEntityCBuf
	{
		struct
		{
			DirectX::XMFLOAT3 ambientCol;
			float p0;
			DirectX::XMFLOAT3 diffuseCol;
			float p1;
			DirectX::XMFLOAT3 specularCol;
			float shininess;

		} material;
	};

	struct TextureVSSystemCBuf
	{
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	struct TextureVSEntityCBuf
	{
		DirectX::XMFLOAT4X4 transform;
	};

	struct TexturePSEntityCBuf
	{
		DirectX::XMFLOAT3 color;
		float pad0;
	};
}

RenderingSystem::RenderingSystem(d3dcore::Scene* scene)
	: System(scene)
{
	m_textureShader = Shader::Create("res/shaders/texture.vs.hlsl", "res/shaders/texture.ps.hlsl");
	m_lightShader = Shader::Create("res/shaders/light.vs.hlsl", "res/shaders/light.ps.hlsl");

	Texture2DDesc defTexDesc = {};
	defTexDesc.width = 1;
	defTexDesc.height = 1;
	
	uint32_t white = 0xffffffff;
	m_defaultTexture = Texture2D::Create(&white, defTexDesc);

	m_lightVSSysCBuf = ConstantBuffer::Create(sizeof(cbufs::LightVSSystemCBuf));
	m_lightVSEntityCBuf = ConstantBuffer::Create(sizeof(cbufs::LightVSEntityCBuf));
	m_lightPSSysCBuf = ConstantBuffer::Create(sizeof(cbufs::LightPSSystemCbuf));
	m_lightPSEntityCBuf = ConstantBuffer::Create(sizeof(cbufs::LightPSEntityCBuf));

	m_textureVSSysCBuf = ConstantBuffer::Create(sizeof(cbufs::TextureVSSystemCBuf));
	m_textureVSEntityCBuf = ConstantBuffer::Create(sizeof(cbufs::TextureVSEntityCBuf));
	m_texturePSEntityCBuf = ConstantBuffer::Create(sizeof(cbufs::TexturePSEntityCBuf));

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

void RenderingSystem::Render(const d3dcore::utils::EditorCamera& camera)
{
	Renderer::ClearBuffer(0.1f, 0.1f, 0.1f, 1.0f);

	Renderer::SetFramebuffer(m_framebuffer);
	ViewportDesc vpDesc = {};
	vpDesc.width = static_cast<float>(m_framebuffer->GetDesc().width);
	vpDesc.height = static_cast<float>(m_framebuffer->GetDesc().height);
	vpDesc.topLeftX = 0.0f;
	vpDesc.topLeftY = 0.0f;
	vpDesc.minDepth = 0.0f;
	vpDesc.maxDepth = 1.0f;
	Renderer::SetViewport(vpDesc);

	Renderer::ClearBuffer(0.1f, 0.1f, 0.1f, 1.0f);

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

				cbufs::LightVSSystemCBuf vsSysCBuf = {};
				XMStoreFloat4x4(&vsSysCBuf.camera.view, XMMatrixTranspose(camera.GetViewMatrix()));
				XMStoreFloat4x4(&vsSysCBuf.camera.projection, XMMatrixTranspose(camera.GetProjectionMatrix()));
				vsSysCBuf.camera.viewPos = camera.GetDesc().position;
				m_lightVSSysCBuf->SetData(&vsSysCBuf);

				XMMATRIX transformMatrix = transform.GetTransform() * GetEntityParentsTransform(relationship);
				cbufs::LightVSEntityCBuf vsEntityCBuf = {};
				XMStoreFloat4x4(&vsEntityCBuf.entity.transformMatrix, XMMatrixTranspose(transformMatrix));
				XMStoreFloat4x4(&vsEntityCBuf.entity.normalMatrix, XMMatrixInverse(nullptr, transformMatrix));
				m_lightVSEntityCBuf->SetData(&vsEntityCBuf);

				cbufs::LightPSEntityCBuf psEntityCBuf = {};
				psEntityCBuf.material.ambientCol = mat.ambientCol;
				psEntityCBuf.material.diffuseCol = mat.diffuseCol;
				psEntityCBuf.material.specularCol = mat.specularCol;
				psEntityCBuf.material.shininess = mat.shininess;
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

				cbufs::TextureVSSystemCBuf vsSysCBuf = {};
				XMStoreFloat4x4(&vsSysCBuf.view, XMMatrixTranspose(camera.GetViewMatrix()));
				XMStoreFloat4x4(&vsSysCBuf.projection, XMMatrixTranspose(camera.GetProjectionMatrix()));
				m_textureVSSysCBuf->SetData(&vsSysCBuf);

				cbufs::TextureVSEntityCBuf vsEntityCBuf = {};
				XMStoreFloat4x4(&vsEntityCBuf.transform, XMMatrixTranspose(transform.GetTransform() * GetEntityParentsTransform(relationship)));
				m_textureVSEntityCBuf->SetData(&vsEntityCBuf);

				cbufs::TexturePSEntityCBuf psEntityCBuf = {};
				psEntityCBuf.color = mat.diffuseCol;
				m_texturePSEntityCBuf->SetData(&psEntityCBuf);
			}

			Renderer::DrawIndexed(mesh.iBuffer->GetCount());
		}
	}

	Renderer::SetFramebuffer(nullptr);
}

void RenderingSystem::SetLigths()
{
	cbufs::LightPSSystemCbuf psSysCBuf = {};
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
