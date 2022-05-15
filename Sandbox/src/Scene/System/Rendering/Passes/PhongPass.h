#pragma once
#include "RenderPass.h"
#include "Scene/System/Rendering/ResourceLibrary.h"

class PhongPass : public RenderPass
{
public:
	PhongPass(d3dcore::Scene* scene);
	virtual void Execute() override;

private:
	void SetLights();

private:
	entt::observer m_dirLights;
	entt::observer m_pointLights;
	entt::observer m_spotLights;
	entt::observer m_renderable;
};