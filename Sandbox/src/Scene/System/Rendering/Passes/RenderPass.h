#pragma once
#include "D3DCore.h"

class RenderPass
{
public:
	virtual ~RenderPass() = default;
	virtual void Execute() = 0;

protected:
	RenderPass(d3dcore::Scene* scene) : m_scene(scene) { }

	d3dcore::Scene* m_scene = nullptr;
};