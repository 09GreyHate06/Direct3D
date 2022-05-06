#include "d3dcpch.h"
#include "RenderTarget.h"
#include "D3DContext.h"

namespace d3dcore
{
	RenderTarget::RenderTarget(const RenderTargetDesc& desc)
		: m_desc(desc)
	{
		Resize(desc.width, desc.height);
	}

	std::shared_ptr<RenderTarget> RenderTarget::Create(const RenderTargetDesc& desc)
	{
		return std::shared_ptr<RenderTarget>(new RenderTarget(desc));
	}

	void RenderTarget::Clear(float r, float g, float b, float a)
	{
		float color[] = { r, g, b, a };
		D3DContext::GetDeviceContext()->ClearRenderTargetView(m_rtv.Get(), color);
	}

#ifdef D3DC_INTERNALS
	void RenderTarget::Bind(const DepthStencil* ds)
	{
		D3DContext::GetDeviceContext()->OMSetRenderTargets(1, m_rtv.GetAddressOf(), ds->GetDSV());
	}
#endif

	void RenderTarget::SetTexture2DSamplerState(const SamplerState& sampler)
	{
		D3D11_SAMPLER_DESC desc = {};
		desc.Filter = sampler.filter;
		desc.AddressU = sampler.addressU;
		desc.AddressV = sampler.addressV;
		desc.AddressW = sampler.addressV;
		desc.MipLODBias = 0.0f;
		desc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
		desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		memcpy(desc.BorderColor, sampler.borderColor, 4 * sizeof(float));
		desc.MinLOD = 0.0f;
		desc.MaxLOD = D3D11_FLOAT32_MAX;
		HRESULT hr;
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateSamplerState(&desc, &m_texSampler));
	}

	void RenderTarget::Resize(uint32_t width, uint32_t height)
	{
		m_desc.width = width;
		m_desc.height = height;

		m_rtv.Reset();
		m_rtTexture.Reset();
		m_rtSrv.Reset();

		bool multisampled = m_desc.samples > 1;

		D3D11_TEXTURE2D_DESC rtTexDesc = {};
		rtTexDesc.Width = m_desc.width;
		rtTexDesc.Height = m_desc.height;
		rtTexDesc.MipLevels = 1;
		rtTexDesc.ArraySize = 1;
		rtTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtTexDesc.SampleDesc.Count = m_desc.samples;
		rtTexDesc.SampleDesc.Quality = m_desc.sampleQuality;
		rtTexDesc.Usage = D3D11_USAGE_DEFAULT;
		rtTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		rtTexDesc.CPUAccessFlags = 0;
		rtTexDesc.MiscFlags = 0;

		HRESULT hr;

		// create render target texture
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateTexture2D(&rtTexDesc, nullptr, &m_rtTexture));
		

		// Setup the description of the render target view.
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = rtTexDesc.Format;
		rtvDesc.ViewDimension = multisampled ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		// Create Render Target View
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateRenderTargetView(m_rtTexture.Get(), &rtvDesc, &m_rtv));


		// Setup the description of the shader resource view.
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = rtTexDesc.Format;
		srvDesc.ViewDimension = multisampled ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture1D.MipLevels = 1;

		// Create the shader resource view.
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateShaderResourceView(m_rtTexture.Get(), &srvDesc, &m_rtSrv));
	}

	void RenderTarget::VSBindAsTexture2D(uint32_t slot) const
	{
		D3DContext::GetDeviceContext()->VSSetSamplers(slot, 1, m_texSampler.GetAddressOf());
		D3DContext::GetDeviceContext()->VSSetShaderResources(slot, 1, m_rtSrv.GetAddressOf());
	}

	void RenderTarget::PSBindAsTexture2D(uint32_t slot) const
	{
		D3DContext::GetDeviceContext()->PSSetSamplers(slot, 1, m_texSampler.GetAddressOf());
		D3DContext::GetDeviceContext()->PSSetShaderResources(slot, 1, m_rtSrv.GetAddressOf());
	}

	void RenderTarget::Resolve(RenderTarget* dst, RenderTarget* src)
	{
		auto dev = D3DContext::GetDevice();
		auto ctx = D3DContext::GetDeviceContext();

		D3DC_CORE_ASSERT(dst->m_desc.samples == 1 && src->m_desc.samples > 1,
			"'dst' must be non-multisampled and 'src' must be multisampled");
		D3DC_CORE_ASSERT(dst->m_desc.width == src->m_desc.width && dst->m_desc.height == src->m_desc.height,
			"Must have same size");

		D3DC_CONTEXT_THROW_INFO_ONLY(ctx->ResolveSubresource(dst->m_rtTexture.Get(), D3D11CalcSubresource(0, 0, 1), src->m_rtTexture.Get(), D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R8G8B8A8_UNORM));
		
		HRESULT hr;

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		D3DC_CONTEXT_THROW_INFO(dev->CreateRenderTargetView(dst->m_rtTexture.Get(), &rtvDesc, &dst->m_rtv));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		D3DC_CONTEXT_THROW_INFO(dev->CreateShaderResourceView(dst->m_rtTexture.Get(), &srvDesc, &dst->m_rtSrv));
	}
}