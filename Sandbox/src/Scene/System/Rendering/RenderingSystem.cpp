#pragma once
#include "RenderingSystem.h"
#include "Scene/Components/Components.h"
#include "Utils/ShaderCBufs.h"
#include "Utils/ComponentUtils.h"
#include "D3DCore/Utils/D3DCMath.h"
#include "ResourceLibrary.h"
#include "D3DCore/Utils/BasicMesh.h"

#include <stack>

using namespace d3dcore;
using namespace d3dcore::utils;
using namespace DirectX;

enum class DepthStencilMode
{
	Default,
	Write,
	Mask,
	DepthOff,
};

static void SetBlendState(bool blend)
{
	D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
	if (blend)
	{
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
	}

	Renderer::SetBlendState(blendDesc);
}

static void SetDepthStencilState(DepthStencilMode mode, uint32_t stencilRef = 0xff)
{
	D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
	switch (mode)
	{
	case DepthStencilMode::Default:
		break;
	case DepthStencilMode::Write:
	{
		dsDesc.DepthEnable = FALSE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.StencilEnable = TRUE;
		dsDesc.StencilReadMask = 0x0;
		dsDesc.StencilWriteMask = 0xff;
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	}
	break;
	case DepthStencilMode::Mask:
	{
		dsDesc.DepthEnable = FALSE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.StencilEnable = TRUE;
		dsDesc.StencilReadMask = 0xff;
		dsDesc.StencilWriteMask = 0x0;
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	}
	break;
	case DepthStencilMode::DepthOff:
	{
		dsDesc.DepthEnable = FALSE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	}
	break;
	}

	Renderer::SetDepthStencilState(dsDesc, stencilRef);
}



RenderingSystem::RenderingSystem(d3dcore::Scene* scene)
	: m_scene(scene)
{
	SetBlendState(true);
	SetDepthStencilState(DepthStencilMode::Default);

	auto& window = Application::Get().GetWindow();
	RenderTargetDesc rtDesc = {};
	rtDesc.width = window.GetWidth();
	rtDesc.height = window.GetHeight();
	rtDesc.samples = 4;
	rtDesc.sampleQuality = 1;
	for (auto& msRT : m_msRenderTargets)
	{
		msRT = RenderTarget::Create(rtDesc);
		//SamplerState sampler = {};
		//sampler.filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		//sampler.addressU = D3D11_TEXTURE_ADDRESS_MIRROR;
		//sampler.addressV = D3D11_TEXTURE_ADDRESS_MIRROR;
		//msRT->SetTexture2DSamplerState(sampler);
	}

	rtDesc.samples = 1;
	rtDesc.sampleQuality = 0;
	m_renderTarget = RenderTarget::Create(rtDesc);
	SamplerState sampler = {};
	sampler.filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.addressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	sampler.addressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	m_renderTarget->SetTexture2DSamplerState(sampler);

	DepthStencilDesc dsDesc = {};
	dsDesc.width = window.GetWidth();
	dsDesc.height = window.GetHeight();
	dsDesc.samples = 4;
	dsDesc.sampleQuality = 1;
	m_msDepthStencil = DepthStencil::Create(dsDesc);


	LoadResources();
	SetBlurCBuf();

	m_observer.connect(m_scene->GetRegistry(), entt::collector.group<MeshComponent, OutlineComponent>(entt::exclude<>));
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
	std::cout << m_observer.size() << '\n';
	SetLigths();

	cbufs::light::VSSystemCBuf vsLightSysCBuf = {};
	XMStoreFloat4x4(&vsLightSysCBuf.camera.view, XMMatrixTranspose(camera.GetViewMatrix()));
	XMStoreFloat4x4(&vsLightSysCBuf.camera.projection, XMMatrixTranspose(camera.GetProjectionMatrix()));
	vsLightSysCBuf.camera.viewPos = camera.GetDesc().position;
	ResourceLibrary<ConstantBuffer>::Get("light_vs_system")->SetData(&vsLightSysCBuf);


	cbufs::basic::VSSystemCBuf vsTexSysCBuf = {};
	XMStoreFloat4x4(&vsTexSysCBuf.view, XMMatrixTranspose(camera.GetViewMatrix()));
	XMStoreFloat4x4(&vsTexSysCBuf.projection, XMMatrixTranspose(camera.GetProjectionMatrix()));
	ResourceLibrary<ConstantBuffer>::Get("basic_vs_system")->SetData(&vsTexSysCBuf);

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
				m_stencilOutlinePass.Add({ transformMatrix, mesh, renderer, Entity(entity, m_scene).GetComponent<OutlineComponent>() });
			}
		}
	}

	Render_();
	RenderTarget::Resolve(m_renderTarget.get(), m_msRenderTargets[0].get());
}

void RenderingSystem::SetRenderTargetSize(uint32_t width, uint32_t height)
{
	for (const auto& msRT : m_msRenderTargets)
		msRT->Resize(width, height);

	m_renderTarget->Resize(width, height);
	m_msDepthStencil->Resize(width, height);
}

void RenderingSystem::Render_()
{
	D3D11_VIEWPORT vp = {};
	vp.Width = (float)m_renderTarget->GetDesc().width;
	vp.Height = (float)m_renderTarget->GetDesc().height;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	Renderer::SetViewport(vp);



	Renderer::SetRenderTarget(m_msRenderTargets[0].get());
	Renderer::SetDepthStencil(m_msDepthStencil.get());
	Renderer::ClearActiveRTV(0.0f, 0.0f, 0.0f, 0.0f);
	Renderer::ClearActiveDSV(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	SetDepthStencilState(DepthStencilMode::Default);
	m_normalPass.Execute();
	SetDepthStencilState(DepthStencilMode::Write);
	m_stencilWritePass.Execute();

	SetBlendState(false);
	Renderer::SetRenderTarget(m_msRenderTargets[1].get());
	Renderer::SetDepthStencil(nullptr);
	Renderer::ClearActiveRTV(0.0f, 0.0f, 0.0f, 0.0f);
	SetDepthStencilState(DepthStencilMode::Default);
	m_stencilOutlinePass.Execute();



	// ------ Post Process ------

	auto blurShader = ResourceLibrary<Shader>::Get("fullscreen_blur");
	blurShader->Bind();
	ResourceLibrary<VertexBuffer>::Get("screen_vb")->Bind();
	ResourceLibrary<IndexBuffer>::Get("screen_ib")->Bind();
	ResourceLibrary<ConstantBuffer>::Get("blur_ps_kernel")->PSBind(blurShader->GetPSResBinding("Kernel"));
	auto blurControl = ResourceLibrary<ConstantBuffer>::Get("blur_ps_control");

	// horizontal pass
	Renderer::SetRenderTarget(m_msRenderTargets[2].get());
	Renderer::SetDepthStencil(nullptr);
	Renderer::ClearActiveRTV(0.0f, 0.0f, 0.0f, 0.0f);

	cbufs::blur::PSControlCBuf controlCbuf;
	controlCbuf.horizontal = TRUE;
	blurControl->SetData(&controlCbuf);
	blurControl->PSBind(blurShader->GetPSResBinding("Control"));
	RenderTarget::Resolve(m_renderTarget.get(), m_msRenderTargets[1].get());
	m_renderTarget->PSBindAsTexture2D(blurShader->GetPSResBinding("tex"));

	Renderer::SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Renderer::DrawIndexed(ResourceLibrary<IndexBuffer>::Get("screen_ib")->GetCount());

	// vertical pass
	Renderer::SetRenderTarget(m_msRenderTargets[0].get());
	Renderer::SetDepthStencil(m_msDepthStencil.get());
	SetDepthStencilState(DepthStencilMode::Mask);
	SetBlendState(true);
	controlCbuf.horizontal = FALSE;
	blurControl->SetData(&controlCbuf);
	blurControl->PSBind(blurShader->GetPSResBinding("Control"));
	RenderTarget::Resolve(m_renderTarget.get(), m_msRenderTargets[2].get());
	m_renderTarget->PSBindAsTexture2D(blurShader->GetPSResBinding("tex"));

	Renderer::SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Renderer::DrawIndexed(ResourceLibrary<IndexBuffer>::Get("screen_ib")->GetCount());

	// ---- Post Process End ----

	Renderer::SetDepthStencil(nullptr);
	Renderer::SetRenderTarget(DEFAULT_RENDER_TARGET);
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

	ResourceLibrary<ConstantBuffer>::Get("light_ps_system")->SetData(&psSysCBuf);
}

void RenderingSystem::SetBlurCBuf()
{
	int radius = 15;
	float sigma = 5.2f;
	cbufs::blur::PSKernelCBuf k;
	k.nTaps = radius * 2 + 1;
	float sum = 0.0f;
	for (int i = 0; i < k.nTaps; i++)
	{
		const float x = i - radius;
		const float gauss = utils::Gauss(x, sigma);
		sum += gauss;
		k.coefficients[i].x = gauss;
	}
	for (int i = 0; i < k.nTaps; i++)
	{
		k.coefficients[i].x /= sum;
	}

	ResourceLibrary<ConstantBuffer>::Get("blur_ps_kernel")->SetData(&k);
}

void RenderingSystem::LoadResources()
{
	// ----- Shaders -----
	ResourceLibrary<Shader>::Add("basic",             Shader::Create("res/shaders/basic.vs.hlsl", "res/shaders/basic.ps.hlsl"));
	ResourceLibrary<Shader>::Add("lighting",          Shader::Create("res/shaders/light.vs.hlsl", "res/shaders/light.ps.hlsl"));
	ResourceLibrary<Shader>::Add("nullps",            Shader::Create("res/shaders/basic.vs.hlsl"));
	ResourceLibrary<Shader>::Add("fullscreen_invert", Shader::Create("res/shaders/fullscreen.vs.hlsl", "res/shaders/invert.ps.hlsl"));
	ResourceLibrary<Shader>::Add("fullscreen_blur",   Shader::Create("res/shaders/fullscreen.vs.hlsl", "res/shaders/blur.ps.hlsl"));
	// --- End Shaders ---

	
	// ----- Texture -----
	d3dcore::Texture2DDesc defTexDesc = {};
	defTexDesc.width = 1;
	defTexDesc.height = 1;
	uint32_t white = 0xffffffff;
	ResourceLibrary<Texture2D>::Add("default", Texture2D::Create(&white, defTexDesc));
	// --- End Texture ---



	// ----- Buffers -----

	float screenVertices[] = {
			-1.0f,  1.0f,
			 1.0f,  1.0f,
			 1.0f, -1.0f,
			-1.0f, -1.0f,
	};

	uint32_t screenIndices[] = {
		0, 1, 2,
		2, 3, 0
	};

	VertexBufferDesc screenVBDesc = {};
	screenVBDesc.size = uint32_t(2 * 4 * sizeof(float));
	screenVBDesc.stride = 2 * sizeof(float);
	screenVBDesc.usage = D3D11_USAGE_DEFAULT;
	screenVBDesc.cpuAccessFlag = 0;
	ResourceLibrary<VertexBuffer>::Add("screen_vb", VertexBuffer::Create(screenVertices, screenVBDesc));

	IndexBufferDesc screenIBDesc = {};
	screenIBDesc.count = 6;
	screenIBDesc.usage = D3D11_USAGE_DEFAULT;
	screenIBDesc.cpuAccessFlag = 0;
	ResourceLibrary<IndexBuffer>::Add("screen_ib", IndexBuffer::Create(screenIndices, screenIBDesc));



	auto cubeVertices = utils::CreateCubeVerticesEx();
	auto cubeIndices = utils::CreateCubeIndicesEx();
	auto planeVertices = utils::CreatePlaneVerticesEx();
	auto planeIndices = utils::CreatePlaneIndicesEx();

	VertexBufferDesc cubeVBDesc = {};
	cubeVBDesc.size = (uint32_t)cubeVertices.size() * sizeof(utils::Vertex);
	cubeVBDesc.stride = sizeof(utils::Vertex);
	cubeVBDesc.usage = D3D11_USAGE_DEFAULT;
	cubeVBDesc.cpuAccessFlag = 0;
	ResourceLibrary<VertexBuffer>::Add("cube_vb", VertexBuffer::Create(cubeVertices.data(), cubeVBDesc));

	IndexBufferDesc cubeIBDesc = {};
	cubeIBDesc.count = (uint32_t)cubeIndices.size();
	cubeIBDesc.usage = D3D11_USAGE_DEFAULT;
	cubeIBDesc.cpuAccessFlag = 0;
	ResourceLibrary<IndexBuffer>::Add("cube_ib", IndexBuffer::Create(cubeIndices.data(), cubeIBDesc));

	VertexBufferDesc planeVBDesc = {};
	planeVBDesc.size = (uint32_t)planeVertices.size() * sizeof(utils::Vertex);
	planeVBDesc.stride = sizeof(utils::Vertex);
	planeVBDesc.usage = D3D11_USAGE_DEFAULT;
	planeVBDesc.cpuAccessFlag = 0;
	ResourceLibrary<VertexBuffer>::Add("plane_vb", VertexBuffer::Create(planeVertices.data(), planeVBDesc));

	IndexBufferDesc planeIBDesc = {};
	planeIBDesc.count = (uint32_t)planeIndices.size();
	planeIBDesc.usage = D3D11_USAGE_DEFAULT;
	planeIBDesc.cpuAccessFlag = 0;
	ResourceLibrary<IndexBuffer>::Add("plane_ib", IndexBuffer::Create(planeIndices.data(), planeIBDesc));



	ResourceLibrary<ConstantBuffer>::Add("light_vs_system", ConstantBuffer::Create(sizeof(cbufs::light::VSSystemCBuf)));
	ResourceLibrary<ConstantBuffer>::Add("light_vs_entity", ConstantBuffer::Create(sizeof(cbufs::light::VSEntityCBuf)));
	ResourceLibrary<ConstantBuffer>::Add("light_ps_system", ConstantBuffer::Create(sizeof(cbufs::light::PSSystemCbuf)));
	ResourceLibrary<ConstantBuffer>::Add("light_ps_entity", ConstantBuffer::Create(sizeof(cbufs::light::PSEntityCBuf)));

	ResourceLibrary<ConstantBuffer>::Add("basic_vs_system", ConstantBuffer::Create(sizeof(cbufs::basic::VSSystemCBuf)));
	ResourceLibrary<ConstantBuffer>::Add("basic_vs_entity", ConstantBuffer::Create(sizeof(cbufs::basic::VSEntityCBuf)));
	ResourceLibrary<ConstantBuffer>::Add("basic_ps_entity", ConstantBuffer::Create(sizeof(cbufs::basic::PSEntityCBuf)));

	ResourceLibrary<ConstantBuffer>::Add("blur_ps_control", ConstantBuffer::Create(sizeof(cbufs::blur::PSControlCBuf)));
	ResourceLibrary<ConstantBuffer>::Add("blur_ps_kernel",  ConstantBuffer::Create(sizeof(cbufs::blur::PSKernelCBuf)));

	// --- End Buffers ---
}
