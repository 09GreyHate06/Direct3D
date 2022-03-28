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

	ComPtr<ID3D11RenderTargetView> Renderer::s_defRTV;
	ComPtr<ID3D11DepthStencilView> Renderer::s_defDSV;

	ViewportDesc Renderer::s_viewport;

	void Renderer::Init(uint32_t sampleCount, uint32_t sampleQuality)
	{
		D3DContext::Init(Application::Get().GetWindow().GetNativeWindow(), sampleCount, sampleQuality);

		s_viewport.width = static_cast<float>(Application::Get().GetWindow().GetWidth());
		s_viewport.height = static_cast<float>(Application::Get().GetWindow().GetHeight());
		s_viewport.topLeftX = 0.0f;
		s_viewport.topLeftY = 0.0f;
		s_viewport.minDepth = 0.0f;
		s_viewport.maxDepth = 1.0f;
		SetViewport(s_viewport);

		s_usingDefRTV = true;
		SetFramebuffer(nullptr);
	}

	void Renderer::Shutdown()
	{
		D3DContext::Shutdown();
	}

	void Renderer::ClearBuffer(float r, float g, float b, float a)
	{
		float color[] = { r, g, b, a };
		ctx::GetDeviceContext()->ClearRenderTargetView(s_activeRTV, color);
		ctx::GetDeviceContext()->ClearDepthStencilView(s_activeDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, s_viewport.maxDepth, 0);
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

	void Renderer::SetTopology(Topology topology)
	{
		ctx::GetDeviceContext()->IASetPrimitiveTopology(static_cast<D3D11_PRIMITIVE_TOPOLOGY>(topology));
	}

	void Renderer::SetViewport(ViewportDesc vp)
	{
		s_viewport = vp;

		if(s_usingDefRTV)
			ResizeDefRTV();

		ctx::GetDeviceContext()->RSSetViewports(1, reinterpret_cast<D3D11_VIEWPORT*>(&s_viewport));
	}

	void Renderer::SetDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& dsDesc)
	{
		HRESULT hr;
		ComPtr<ID3D11DepthStencilState> dsState = nullptr;
		D3DC_CONTEXT_THROW_INFO(ctx::GetDevice()->CreateDepthStencilState(&dsDesc, &dsState));
		ctx::GetDeviceContext()->OMSetDepthStencilState(dsState.Get(), 0xff);
	}

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
		ctx::GetDeviceContext()->OMSetBlendState(blendState.Get(), factor.data(), 0xffffffff);
	}

	void Renderer::SetRasterizerState(const D3D11_RASTERIZER_DESC& rsDesc)
	{
		HRESULT hr;
		ComPtr<ID3D11RasterizerState> rsState = nullptr;
		D3DC_CONTEXT_THROW_INFO(ctx::GetDevice()->CreateRasterizerState(&rsDesc, &rsState));
		ctx::GetDeviceContext()->RSSetState(rsState.Get());
	}

	void Renderer::SetFramebuffer(const std::shared_ptr<Framebuffer>& fb)
	{
		s_activeRTV = fb ? fb->m_renderTargetView.Get() : s_defRTV.Get();
		s_activeDSV = fb ? fb->m_depthStencilView.Get() : s_defDSV.Get();
		s_usingDefRTV = fb ? false : true;

		ctx::GetDeviceContext()->OMSetRenderTargets(1, &s_activeRTV, s_activeDSV);
	}

	void Renderer::ResizeDefRTV()
	{
		s_defRTV.Reset();
		s_defDSV.Reset();

		ctx::GetSwapChain()->ResizeBuffers(1, static_cast<uint32_t>(s_viewport.width), static_cast<uint32_t>(s_viewport.height), DXGI_FORMAT_R8G8B8A8_UNORM, 0);

		HRESULT hr;

		// gain access to texturesub resource in swap chain (back buffer)
		ComPtr<ID3D11Resource> backBuffer = nullptr;
		D3DC_CONTEXT_THROW_INFO(ctx::GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer)); // 0 is the index of back buffer
		D3DC_CONTEXT_THROW_INFO(ctx::GetDevice()->CreateRenderTargetView(backBuffer.Get(), nullptr, &s_defRTV));

		// create depth stencil texture
		ComPtr<ID3D11Texture2D> dsTexture = nullptr;
		D3D11_TEXTURE2D_DESC dsTextureDesc = {};
		dsTextureDesc.Width = static_cast<uint32_t>(s_viewport.width);
		dsTextureDesc.Height = s_viewport.height ? static_cast<uint32_t>(s_viewport.height) : 1;
		dsTextureDesc.MipLevels = 1;
		dsTextureDesc.ArraySize = 1;
		dsTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsTextureDesc.SampleDesc.Count = D3DContext::GetSampleCount();
		dsTextureDesc.SampleDesc.Quality = D3DContext::GetSampleQuality();
		dsTextureDesc.Usage = D3D11_USAGE_DEFAULT;
		dsTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		D3DC_CONTEXT_THROW_INFO(ctx::GetDevice()->CreateTexture2D(&dsTextureDesc, nullptr, &dsTexture));

		// create view of depth stencil texture
		D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
		dsViewDesc.Format = dsTextureDesc.Format;
		dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		dsViewDesc.Texture2D.MipSlice = 0;
		D3DC_CONTEXT_THROW_INFO(ctx::GetDevice()->CreateDepthStencilView(dsTexture.Get(), &dsViewDesc, &s_defDSV));

		D3D11_RASTERIZER_DESC rsDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
		SetRasterizerState(rsDesc);

		// bind view to om
		if (s_usingDefRTV)
			SetFramebuffer(nullptr);
	}
}