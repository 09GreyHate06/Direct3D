#pragma once
#include "D3DCore.h"
#include "Utils/GlobalAsset.h"

class TestPass
{
public:
	void Execute(const d3dcore::Framebuffer* fb)
	{
		auto shader = GlobalAsset::GetShader("fullscreen_blur");
		//auto shader = GlobalAsset::GetShader("fullscreen_invert");

		shader->Bind();
		GlobalAsset::GetVertexBuffer("screen_vb")->Bind();
		GlobalAsset::GetIndexBuffer("screen_ib")->Bind();

		//GlobalAsset::GetTexture("default")->PSBind(shader->GetPSResBinding("tex"));
		fb->PSBindColorAttAsTexture2D(shader->GetPSResBinding("tex"));

		d3dcore::Renderer::SetTopology(d3dcore::Topology::TriangleList);
		d3dcore::Renderer::DrawIndexed(GlobalAsset::GetIndexBuffer("screen_ib")->GetCount());
	}

private:

};