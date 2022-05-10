#pragma once
#include "D3DCore.h"
#include "D3DCore/Utils/Camera.h"
#include "Passes/NormalPass.h"
#include "Passes/StencilWritePass.h"
#include "Passes/StencilOutlineEffectPass.h"
#include "Passes/TestPass.h"
#include <queue>

class RenderingSystem
{
public:
	RenderingSystem(d3dcore::Scene* scene);
	RenderingSystem();

	void SetScene(d3dcore::Scene* scene);
	void Render(const d3dcore::utils::Camera& camera);
	const std::shared_ptr<d3dcore::RenderTarget>& GetRenderTarget() const { return m_renderTarget; }

	void SetRenderTargetSize(uint32_t width, uint32_t height);

private:
	d3dcore::Scene* m_scene;

	void Render_();

	void SetLigths();
	void SetBlurCBuf();
	void LoadResources();

	NormalPass m_normalPass;
	StencilWritePass m_stencilWritePass;
	StencilOutlineEffectPass m_stencilOutlinePass;
	//TestPass m_testPass;

	std::array<std::shared_ptr<d3dcore::RenderTarget>, 3> m_msRenderTargets;
	std::shared_ptr<d3dcore::DepthStencil> m_msDepthStencil;

	std::shared_ptr<d3dcore::RenderTarget> m_renderTarget;

	entt::observer m_observer;
};