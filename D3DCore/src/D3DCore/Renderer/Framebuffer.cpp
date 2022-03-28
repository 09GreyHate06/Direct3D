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

	std::shared_ptr<Framebuffer> Framebuffer::Create(const FramebufferDesc& desc)
	{
		return std::shared_ptr<Framebuffer>(new Framebuffer(desc));
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

		D3D11_TEXTURE2D_DESC rtTexDesc = {};
		rtTexDesc.Width = m_desc.width;
		rtTexDesc.Height = m_desc.height;
		rtTexDesc.MipLevels = 1;
		rtTexDesc.ArraySize = 1;
		rtTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtTexDesc.SampleDesc.Count = D3DContext::GetSampleCount();
		rtTexDesc.SampleDesc.Quality = D3DContext::GetSampleQuality();
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
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
		rtvDesc.Texture2D.MipSlice = 0;

		// Create the render target view.
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateRenderTargetView(m_renderTargetTexture.Get(), &rtvDesc, &m_renderTargetView));

		// Setup the description of the shader resource view.
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = rtTexDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		// Create the shader resource view.
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateShaderResourceView(m_renderTargetTexture.Get(), &srvDesc, &m_renderTargetSrv));


		D3D11_TEXTURE2D_DESC dsTexDesc = {};
		dsTexDesc.Width = m_desc.width;
		dsTexDesc.Height = m_desc.height;
		dsTexDesc.MipLevels = 1;
		dsTexDesc.ArraySize = 1;
		dsTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsTexDesc.SampleDesc.Count = D3DContext::GetSampleCount();
		dsTexDesc.SampleDesc.Quality = D3DContext::GetSampleQuality();
		dsTexDesc.Usage = D3D11_USAGE_DEFAULT;
		dsTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		// create depth stencil texture
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateTexture2D(&dsTexDesc, nullptr, &m_depthStencilTexture));

		// create view of depth stencil texture
		D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
		dsViewDesc.Format = dsTexDesc.Format;
		dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		dsViewDesc.Texture2D.MipSlice = 0;
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateDepthStencilView(m_depthStencilTexture.Get(), &dsViewDesc, &m_depthStencilView));
	}
}