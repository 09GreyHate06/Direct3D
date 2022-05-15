#pragma once

#include "D3DCore.h"
#include "RenderPass.h"

class StencilWritePass : public RenderPass
{
public:
	StencilWritePass(d3dcore::Scene* scene);
	virtual void Execute() override;

private:
	entt::observer m_entities;
};