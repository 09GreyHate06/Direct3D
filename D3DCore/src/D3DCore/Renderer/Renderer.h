#pragma once
#include "D3DCore/Core/D3DCWindows.h"
#include "RenderTarget.h"
#include "DepthStencil.h"
#include <d3d11.h>
#include <optional>

namespace d3dcore
{
	class Renderer
	{
	public:
		static void ClearActiveRTV(float r, float g, float b, float a);
		static void ClearActiveDSV(uint32_t clearFlags, float depth, uint8_t stencil);
		static void SwapBuffers(uint32_t syncInterval);
		static void DrawIndexed(uint32_t count);

		static void SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
		static void SetViewport(const D3D11_VIEWPORT& vp);
		static void SetDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& dsDesc, uint32_t stencilRef = 0xff);
		static void SetBlendState(const D3D11_BLEND_DESC& bsDesc, std::optional<float> blendFactor = {});
		static void SetRasterizerState(const D3D11_RASTERIZER_DESC& rsDesc);

		static void SetRenderTarget(const RenderTarget* rt);
		static void SetDepthStencil(const DepthStencil* ds);

		static const D3D11_VIEWPORT& GetViewport() { return s_viewport; }


	private:
		static void Init();
		static void Shutdown();
		static void ResizeSwapChainRTV();

		static std::vector<Microsoft::WRL::ComPtr<ID3D11BlendState>> m_blendStates;
		static std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilState>> m_depthStencilStates;

		static ID3D11RenderTargetView* s_activeRTV;
		static ID3D11DepthStencilView* s_activeDSV;
		static bool s_usingDefRTV;

		static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> s_swapChainRTV;

		static D3D11_VIEWPORT s_viewport;

		friend class Application;
	};
}

#define DEFAULT_RENDER_TARGET nullptr