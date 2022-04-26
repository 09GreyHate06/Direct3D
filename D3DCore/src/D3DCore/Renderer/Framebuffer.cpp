#include "d3dcpch.h"
#include "Framebuffer.h"
#include "D3DContext.h"

namespace d3dcore
{
	Framebuffer::Framebuffer(const FramebufferDesc& desc)
		: m_desc(desc)
	{
		Resize(desc.width, desc.height);
	}

	//void Framebuffer::Blit(Framebuffer* dst, Framebuffer* src)
	//{
	//	//Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	//	//Microsoft::WRL::ComPtr<ID3D11Texture2D> m_renderTargetTexture;
	//	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_renderTargetSrv;
	//	//Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	//	//Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilTexture;

	//	bool dstMS = dst->m_desc.samples > 1;
	//	bool srcMS = src->m_desc.samples > 1;

	//	auto dev = D3DContext::GetDevice();
	//	auto ctx = D3DContext::GetDeviceContext();

	//	D3D11_TEXTURE2D_DESC rtTexDesc = {};
	//	rtTexDesc.Width = src->m_desc.width;
	//	rtTexDesc.Height = src->m_desc.height;
	//	rtTexDesc.MipLevels = 1;
	//	rtTexDesc.ArraySize = 1;
	//	rtTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//	rtTexDesc.SampleDesc.Count = dst->m_desc.samples;
	//	rtTexDesc.SampleDesc.Quality = dst->m_desc.sampleQuality;
	//	rtTexDesc.Usage = D3D11_USAGE_DEFAULT;
	//	rtTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	//	rtTexDesc.CPUAccessFlags = 0;
	//	rtTexDesc.MiscFlags = 0;

	//	HRESULT hr;

	//	// create render target texture
	//	D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateTexture2D(&rtTexDesc, nullptr, &dst->m_renderTargetTexture));
	//	

	//	if (!dstMS && srcMS)
	//	{
	//		D3DC_CONTEXT_THROW_INFO_ONLY(ctx->ResolveSubresource(dst->m_renderTargetTexture.Get(), 0, src->m_renderTargetTexture.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM));
	//	}
	//	else
	//	{
	//		D3DC_CONTEXT_THROW_INFO_ONLY(ctx->CopyResource(dst->m_renderTargetTexture.Get(), src->m_renderTargetTexture.Get()));
	//	}


	//	// Setup the description of the render target view.
	//	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	//	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//	rtvDesc.ViewDimension = dstMS ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
	//	rtvDesc.Texture2D.MipSlice = 0;
	//	D3DC_CONTEXT_THROW_INFO(dev->CreateRenderTargetView(dst->m_renderTargetTexture.Get(), &rtvDesc, &dst->m_renderTargetView));

	//	// Setup the description of the shader resource view.
	//	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//	srvDesc.Format = rtvDesc.Format;
	//	srvDesc.ViewDimension = dstMS ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
	//	srvDesc.Texture2D.MostDetailedMip = 0;
	//	srvDesc.Texture2D.MipLevels = 1;
	//	D3DC_CONTEXT_THROW_INFO(dev->CreateShaderResourceView(dst->m_renderTargetTexture.Get(), &srvDesc, &dst->m_renderTargetSrv));


	//	if (dst->m_desc.hasDepth && src->m_desc.hasDepth)
	//	{
	//		D3D11_TEXTURE2D_DESC dsTexDesc = {};
	//		dsTexDesc.Width = src->m_desc.width;
	//		dsTexDesc.Height = src->m_desc.height;
	//		dsTexDesc.MipLevels = 1;
	//		dsTexDesc.ArraySize = 1;
	//		dsTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//		dsTexDesc.SampleDesc.Count = dst->m_desc.samples;
	//		dsTexDesc.SampleDesc.Quality = dst->m_desc.sampleQuality;
	//		dsTexDesc.Usage = D3D11_USAGE_DEFAULT;
	//		dsTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	//		// create depth stencil texture
	//		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateTexture2D(&dsTexDesc, nullptr, &dst->m_depthStencilTexture));

	//		if (!dstMS && srcMS)
	//		{
	//			D3DC_CONTEXT_THROW_INFO_ONLY(ctx->ResolveSubresource(dst->m_depthStencilTexture.Get(), 0, src->m_depthStencilTexture.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM));
	//		}
	//		else
	//		{
	//			D3DC_CONTEXT_THROW_INFO_ONLY(ctx->CopyResource(dst->m_depthStencilTexture.Get(), src->m_depthStencilTexture.Get()));
	//		}

	//		// create view of depth stencil texture
	//		D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
	//		dsViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//		dsViewDesc.ViewDimension = dstMS ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	//		dsViewDesc.Texture2D.MipSlice = 0;
	//		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateDepthStencilView(dst->m_depthStencilTexture.Get(), &dsViewDesc, &dst->m_depthStencilView));

	//		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//		srvDesc.Format = rtvDesc.Format;
	//		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	//		srvDesc.Texture2D.MostDetailedMip = 0;
	//		srvDesc.Texture2D.MipLevels = 1;
	//		D3DC_CONTEXT_THROW_INFO(dev->CreateShaderResourceView(dst->m_renderTargetTexture.Get(), &srvDesc, &dst->m_renderTargetSrv));
	//	}
	//}

	void Framebuffer::Resolve(Framebuffer* dst, Framebuffer* src)
	{
		auto dev = D3DContext::GetDevice();
		auto ctx = D3DContext::GetDeviceContext();

		D3D11_TEXTURE2D_DESC rtTexDesc = {};
		rtTexDesc.Width = src->m_desc.width;
		rtTexDesc.Height = src->m_desc.height;
		rtTexDesc.MipLevels = 1;
		rtTexDesc.ArraySize = 1;
		rtTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtTexDesc.SampleDesc.Count = dst->m_desc.samples;
		rtTexDesc.SampleDesc.Quality = dst->m_desc.sampleQuality;
		rtTexDesc.Usage = D3D11_USAGE_DEFAULT;
		rtTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;  
		rtTexDesc.CPUAccessFlags = 0;
		rtTexDesc.MiscFlags = 0;

		dst->m_desc.width = src->m_desc.width;
		dst->m_desc.height = src->m_desc.height;

		HRESULT hr;

		D3DC_CONTEXT_THROW_INFO(dev->CreateTexture2D(&rtTexDesc, nullptr, &dst->m_renderTargetTexture));

		D3DC_CONTEXT_THROW_INFO_ONLY(ctx->ResolveSubresource(dst->m_renderTargetTexture.Get(), D3D11CalcSubresource(0, 0, 1), src->m_renderTargetTexture.Get(), D3D11CalcSubresource(0, 0, 1), rtTexDesc.Format));

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = rtTexDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		D3DC_CONTEXT_THROW_INFO(dev->CreateRenderTargetView(dst->m_renderTargetTexture.Get(), &rtvDesc, &dst->m_renderTargetView));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		D3DC_CONTEXT_THROW_INFO(dev->CreateShaderResourceView(dst->m_renderTargetTexture.Get(), &srvDesc, &dst->m_renderTargetSrv));

		// todo How to copy multsampled depth stencil texture
		
		//if (dst->m_desc.hasDepth && src->m_desc.hasDepth)
		//{
		//	D3D11_TEXTURE2D_DESC dsTexDesc = {};
		//	dsTexDesc.Width = src->m_desc.width;
		//	dsTexDesc.Height = src->m_desc.height;
		//	dsTexDesc.MipLevels = 1;
		//	dsTexDesc.ArraySize = 1;
		//	dsTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		//	dsTexDesc.SampleDesc.Count = dst->m_desc.samples;
		//	dsTexDesc.SampleDesc.Quality = dst->m_desc.sampleQuality;
		//	dsTexDesc.Usage = D3D11_USAGE_DEFAULT;
		//	dsTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		//	// create depth stencil texture
		//	D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateTexture2D(&dsTexDesc, nullptr, &dst->m_depthStencilTexture));

		//	D3DC_CONTEXT_THROW_INFO_ONLY(ctx->CopyResource(dst->m_depthStencilTexture.Get(), src->m_depthStencilTexture.Get()));

		//	// create view of depth stencil texture
		//	D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
		//	dsViewDesc.Format = dsTexDesc.Format;
		//	dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		//	dsViewDesc.Texture2D.MipSlice = 0;
		//	D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateDepthStencilView(dst->m_depthStencilTexture.Get(), &dsViewDesc, &dst->m_depthStencilView));
		//}
	}

	std::shared_ptr<Framebuffer> Framebuffer::Create(const FramebufferDesc& desc)
	{
		return std::shared_ptr<Framebuffer>(new Framebuffer(desc));
	}

	void Framebuffer::SetColorAttSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressU, D3D11_TEXTURE_ADDRESS_MODE addressV, float borderColor[4])
	{
		D3D11_SAMPLER_DESC desc = {};
		desc.Filter = filter;
		desc.AddressU = addressU;
		desc.AddressV = addressV;
		desc.AddressW = addressV;
		desc.MipLODBias = 0.0f;
		desc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
		desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		if(borderColor)
			memcpy(desc.BorderColor, borderColor, 4 * sizeof(float));
		desc.MinLOD = 0.0f;
		desc.MaxLOD = D3D11_FLOAT32_MAX;
		HRESULT hr;
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateSamplerState(&desc, &m_colorAttSampler));
	}

	void Framebuffer::Resize(uint32_t width, uint32_t height)
	{
		m_desc.width = width;
		m_desc.height = height;

		m_renderTargetView.Reset();
		m_renderTargetTexture.Reset();
		m_renderTargetSrv.Reset();
		m_depthStencilView.Reset();
		m_depthStencilTexture.Reset();

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
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateTexture2D(&rtTexDesc, nullptr, &m_renderTargetTexture));

		// Setup the description of the render target view.
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = rtTexDesc.Format;
		rtvDesc.ViewDimension = multisampled ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		// Create the render target view.
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateRenderTargetView(m_renderTargetTexture.Get(), &rtvDesc, &m_renderTargetView));

		// Setup the description of the shader resource view.
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = rtTexDesc.Format;
		srvDesc.ViewDimension = multisampled ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		// Create the shader resource view.
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateShaderResourceView(m_renderTargetTexture.Get(), &srvDesc, &m_renderTargetSrv));


		if (m_desc.hasDepth)
		{
			D3D11_TEXTURE2D_DESC dsTexDesc = {};
			dsTexDesc.Width = m_desc.width;
			dsTexDesc.Height = m_desc.height;
			dsTexDesc.MipLevels = 1;
			dsTexDesc.ArraySize = 1;
			dsTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			dsTexDesc.SampleDesc.Count = m_desc.samples;
			dsTexDesc.SampleDesc.Quality = m_desc.sampleQuality;
			dsTexDesc.Usage = D3D11_USAGE_DEFAULT;
			dsTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

			// create depth stencil texture
			D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateTexture2D(&dsTexDesc, nullptr, &m_depthStencilTexture));

			// create view of depth stencil texture
			D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
			dsViewDesc.Format = dsTexDesc.Format;
			dsViewDesc.ViewDimension = multisampled ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
			dsViewDesc.Texture2D.MipSlice = 0;
			D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateDepthStencilView(m_depthStencilTexture.Get(), &dsViewDesc, &m_depthStencilView));
		}
	}

	void Framebuffer::VSBindColorAttAsTexture2D(uint32_t slot) const
	{
		D3DContext::GetDeviceContext()->VSSetSamplers(slot, 1, m_colorAttSampler.GetAddressOf());
		D3DContext::GetDeviceContext()->VSSetShaderResources(slot, 1, m_renderTargetSrv.GetAddressOf());
	}

	void Framebuffer::PSBindColorAttAsTexture2D(uint32_t slot) const
	{
		D3DContext::GetDeviceContext()->PSSetSamplers(slot, 1, m_colorAttSampler.GetAddressOf());
		D3DContext::GetDeviceContext()->PSSetShaderResources(slot, 1, m_renderTargetSrv.GetAddressOf());
	}
}