#pragma once
#include "D3DCore.h"
#include "RenderPass.h"

class StencilOutlineEffectPass : public RenderPass
{
public:
	StencilOutlineEffectPass(d3dcore::Scene* scene);

	virtual void Execute() override;

private:
	entt::observer m_entities;
};