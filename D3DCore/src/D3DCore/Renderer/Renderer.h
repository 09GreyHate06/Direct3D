#pragma once
#include "D3DCore/Core/D3DCWindows.h"
#include "Framebuffer.h"
#include <d3d11.h>
#include <optional>

namespace d3dcore
{
	enum class Topology
	{
		PointList = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
		LineList = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		LineStrip = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
		TriangleList = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		TriangleStrip = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
	};

	class Renderer
	{
	public:

		static void ClearBuffer(float r, float g, float b, float a);
		static void SwapBuffers(uint32_t syncInterval);
		static void DrawIndexed(uint32_t count);

		static void SetTopology(Topology topology);
		static void SetViewport(const D3D11_VIEWPORT& vp);
		static void SetDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& dsDesc, uint32_t stencilRef = 1);
		static void SetBlendState(const D3D11_BLEND_DESC& bsDesc, std::optional<float> blendFactor = {});
		static void SetRasterizerState(const D3D11_RASTERIZER_DESC& rsDesc);

		// SetFramebuffer doesn't increment shared ptr ref count
		static void SetFramebuffer(const std::shared_ptr<Framebuffer>& fb);

		static const D3D11_VIEWPORT& GetViewport() { return s_viewport; }


	private:
		static void Init();
		static void Shutdown();
		static void ResizeDefRTV();

		static ID3D11RenderTargetView* s_activeRTV;
		static ID3D11DepthStencilView* s_activeDSV;
		static bool s_usingDefRTV;

		static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> s_defRTV;
		static Microsoft::WRL::ComPtr<ID3D11DepthStencilView> s_defDSV;

		static D3D11_VIEWPORT s_viewport;

		friend class Application;
	};
}