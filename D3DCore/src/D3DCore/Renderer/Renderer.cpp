#include "d3dcpch.h"
#include "Renderer.h"
#include "D3DContext.h"
#include "D3DCore/Core/Application.h"

using namespace Microsoft::WRL;

namespace d3dcore
{
	using ctx = D3DContext;

	ID3D11RenderTargetView* Renderer::s_activeRTV = nullptr;
	ID3D11DepthStencilView* Renderer::s_activeDSV = nullptr;
	bool Renderer::s_usingDefRTV = true;

	ComPtr<ID3D11RenderTargetView> Renderer::s_swapChainRTV;

	D3D11_VIEWPORT Renderer::s_viewport;

	void Renderer::Init()
	{
		D3DContext::Init(Application::Get().GetWindow().GetNativeWindow());

		D3D11_VIEWPORT vp = {};
		vp.Width = static_cast<float>(Application::Get().GetWindow().GetWidth());
		vp.Height = static_cast<float>(Application::Get().GetWindow().GetHeight());
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		SetViewport(vp);

		s_usingDefRTV = true;
		SetRenderTarget(nullptr);
	}

	void Renderer::Shutdown()
	{
		D3DContext::Shutdown();
	}

	void Renderer::ClearActiveRTV(float r, float g, float b, float a)
	{
		D3DC_CORE_ASSERT(s_activeRTV, "No active render target view");
		float color[] = { r, g, b, a };
		ctx::GetDeviceContext()->ClearRenderTargetView(s_activeRTV, color);
	}

	void Renderer::ClearActiveDSV(uint32_t clearFlags, float depth, uint8_t stencil)
	{
		D3DC_CORE_ASSERT(s_activeDSV, "No active depth stencil view");
		ctx::GetDeviceContext()->ClearDepthStencilView(s_activeDSV, clearFlags, depth, stencil);
	}

	void Renderer::SwapBuffers(uint32_t syncInterval)
	{
		HRESULT hr;
		if (FAILED(hr = ctx::GetSwapChain()->Present(syncInterval, 0)))
		{
			if (hr == DXGI_ERROR_DEVICE_REMOVED)
				throw D3DC_CONTEXT_DEVICE_REMOVED_EXCEPT(hr);
			else
				throw D3DC_CONTEXT_EXCEPT(hr);
		}
	}

	void Renderer::DrawIndexed(uint32_t count)
	{
		D3DContext::GetDeviceContext()->OMSetRenderTargets(1, &s_activeRTV, s_activeDSV);
		D3DC_CONTEXT_THROW_INFO_ONLY(ctx::GetDeviceContext()->DrawIndexed(count, 0, 0));
	}

	void Renderer::SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
	{
		ctx::GetDeviceContext()->IASetPrimitiveTopology(topology);
	}

	void Renderer::SetViewport(const D3D11_VIEWPORT& vp)
	{
		auto old = s_viewport;
		s_viewport = vp;

		if (s_usingDefRTV && (old.Width != s_viewport.Width || old.Height != s_viewport.Height))
		{
			ResizeSwapChainRTV();
		}

		ctx::GetDeviceContext()->RSSetViewports(1, &s_viewport);
	}

	void Renderer::SetDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& dsDesc, uint32_t stencilRef)
	{
		HRESULT hr;
		ComPtr<ID3D11DepthStencilState> dsState = nullptr;
		D3DC_CONTEXT_THROW_INFO(ctx::GetDevice()->CreateDepthStencilState(&dsDesc, &dsState));
		ctx::GetDeviceContext()->OMSetDepthStencilState(dsState.Get(), stencilRef);
	}

	//void Renderer::SetDepthStencilState(DepthStencilMode mode, uint32_t stencilRef)
	//{
	//	D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
	//	switch (mode)
	//	{
	//	case d3dcore::DepthStencilMode::Default:
	//		break;
	//	case d3dcore::DepthStencilMode::Write:
	//	{
	//		dsDesc.DepthEnable = FALSE;
	//		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	//		dsDesc.StencilEnable = TRUE;
	//		dsDesc.StencilReadMask = 0x0;
	//		dsDesc.StencilWriteMask = 0xff;
	//		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	//		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	//	}
	//		break;
	//	case d3dcore::DepthStencilMode::Mask:
	//	{
	//		dsDesc.DepthEnable = FALSE;
	//		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	//		dsDesc.StencilEnable = TRUE;
	//		dsDesc.StencilReadMask = 0xff;
	//		dsDesc.StencilWriteMask = 0x0;
	//		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	//		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	//	}
	//		break;
	//	case d3dcore::DepthStencilMode::DepthOff:
	//	{
	//		dsDesc.DepthEnable = FALSE;
	//		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	//	}
	//		break;
	//	}
	//
	//	SetDepthStencilState(dsDesc, stencilRef);
	//}

	void Renderer::SetBlendState(const D3D11_BLEND_DESC& bsDesc, std::optional<float> blendFactor)
	{
		HRESULT hr;
		ComPtr<ID3D11BlendState> blendState = nullptr;
		D3DC_CONTEXT_THROW_INFO(ctx::GetDevice()->CreateBlendState(&bsDesc, &blendState));
		std::array<float, 4> factor = { 0.0f, 0.0f, 0.0f, 0.0f };
		if (blendFactor)
		{
			std::fill(factor.begin(), factor.end(), blendFactor.value());
		}

		float* factorData = blendFactor ? factor.data() : nullptr;
		ctx::GetDeviceContext()->OMSetBlendState(blendState.Get(), factorData, 0xffffffff);
	}

	//void Renderer::SetBlendState(bool blend, std::optional<float> blendFactor)
	//{
	//	D3D11_BLEND_DESC bsDesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
	//
	//	if (blend)
	//	{
	//		for (int i = 0; i < 8; i++)
	//		{
	//			auto& brt = bsDesc.RenderTarget[i];
	//			brt.BlendEnable = TRUE;
	//			brt.SrcBlendAlpha = D3D11_BLEND_ONE; // to preserve alpha value
	//			brt.DestBlendAlpha = D3D11_BLEND_ONE; // to preserve alpha value
	//
	//			if (blendFactor)
	//			{
	//				brt.SrcBlend = D3D11_BLEND_BLEND_FACTOR;
	//				brt.DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
	//			}
	//			else
	//			{
	//				brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	//				brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	//			}
	//		}
	//	}
	//
	//	SetBlendState(bsDesc, blendFactor);
	//}

	void Renderer::SetRasterizerState(const D3D11_RASTERIZER_DESC& rsDesc)
	{
		HRESULT hr;
		ComPtr<ID3D11RasterizerState> rsState = nullptr;
		D3DC_CONTEXT_THROW_INFO(ctx::GetDevice()->CreateRasterizerState(&rsDesc, &rsState));
		ctx::GetDeviceContext()->RSSetState(rsState.Get());
	}

	void Renderer::SetRenderTarget(const RenderTarget* rt)
	{
		s_activeRTV = rt ? rt->m_rtv.Get() : s_swapChainRTV.Get();
		s_usingDefRTV = s_activeRTV == s_swapChainRTV.Get();

		ctx::GetDeviceContext()->OMSetRenderTargets(1, &s_activeRTV, s_activeDSV);
	}

	void Renderer::SetDepthStencil(const DepthStencil* ds)
	{
		s_activeDSV = ds ? ds->m_dsv.Get() : nullptr;
		ctx::GetDeviceContext()->OMSetRenderTargets(1, &s_activeRTV, s_activeDSV);
	}

	void Renderer::ResizeSwapChainRTV()
	{
		s_swapChainRTV.Reset();

		ctx::GetSwapChain()->ResizeBuffers(1, static_cast<uint32_t>(s_viewport.Width), static_cast<uint32_t>(s_viewport.Height), DXGI_FORMAT_R8G8B8A8_UNORM, 0);

		HRESULT hr;

		// gain access to texturesub resource in swap chain (back buffer)
		ComPtr<ID3D11Texture2D> backBuffer = nullptr;
		D3DC_CONTEXT_THROW_INFO(ctx::GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer)); // 0 is the index of back buffer
		D3DC_CONTEXT_THROW_INFO(ctx::GetDevice()->CreateRenderTargetView(backBuffer.Get(), nullptr, &s_swapChainRTV));

		D3D11_RASTERIZER_DESC rsDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
		SetRasterizerState(rsDesc);

		// bind view to om
		if (s_usingDefRTV)
		{
			SetRenderTarget(nullptr);
		}
	}
}