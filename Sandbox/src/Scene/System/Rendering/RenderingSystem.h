#pragma once
#define D3DC_INTERNALS
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
	const std::shared_ptr<d3dcore::Framebuffer> GetFramebuffer() const { return m_framebuffer; }

	void SetViewport(float viewportWidth, float viewportHeight);

private:
	d3dcore::Scene* m_scene;

	void Render_();

	void SetLigths();
	void SetBlurCBuf();

	NormalPass m_normalPass;
	StencilWritePass m_stencilWritePass;
	StencilOutlineEffectPass m_stencilOutlinePass;
	TestPass m_testPass;

	std::array<std::shared_ptr<d3dcore::Framebuffer>, 3> m_msFramebuffers;
	std::shared_ptr<d3dcore::Framebuffer> m_framebuffer;
};