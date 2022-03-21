#pragma once
#include "D3DCore/Core/D3DCWindows.h"
#include "Framebuffer.h"
#include <d3d11.h>

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

	struct ViewportDesc
	{
		float topLeftX = 0.0f;
		float topLeftY = 0.0f;
		float width = 0.0f;
		float height = 0.0f; 
		float minDepth = 0.0f;
		float maxDepth = 1.0f;
	};

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void ClearBuffer(float r, float g, float b, float a);
		static void SwapBuffers(uint32_t syncInterval);
		static void DrawIndexed(uint32_t count);

		static void SetTopology(Topology topology);
		static void SetViewport(ViewportDesc vp);
		static void SetDepthStencilState(D3D11_DEPTH_STENCIL_DESC dsDesc);

		// SetFramebuffer doesn't increment shared ptr ref count
		static void SetFramebuffer(const std::shared_ptr<Framebuffer>& fb);

		static const ViewportDesc& GetViewport() { return s_viewport; }

	private:
		static void ResizeDefRTV();

		static ID3D11RenderTargetView* s_activeRTV;
		static ID3D11DepthStencilView* s_activeDSV;
		static bool s_usingDefRTV;

		static Microsoft::WRL::ComPtr<ID3D11DepthStencilState> s_depthStencilState;
		static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> s_defRTV;
		static Microsoft::WRL::ComPtr<ID3D11DepthStencilView> s_defDSV;

		static ViewportDesc s_viewport;
	};
}