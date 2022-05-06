#include "d3dcpch.h"
#include "DepthStencil.h"
#include "D3DContext.h"

namespace d3dcore
{
	DepthStencil::DepthStencil(const DepthStencilDesc& desc)
		:m_desc(desc)
	{
		Resize(desc.width, desc.height);
	}

	std::shared_ptr<DepthStencil> DepthStencil::Create(const DepthStencilDesc& desc)
	{
		return std::shared_ptr<DepthStencil>(new DepthStencil(desc));
	}

	void DepthStencil::Clear(uint32_t clearFlags, float depth, uint8_t stencil)
	{
		D3DContext::GetDeviceContext()->ClearDepthStencilView(m_dsv.Get(), clearFlags, depth, stencil);
	}


	void DepthStencil::Resize(uint32_t width, uint32_t height)
	{
		m_desc.width = width;
		m_desc.height = height;

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

		HRESULT hr;

		// create depth stecil texture
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateTexture2D(&dsTexDesc, nullptr, &m_dsTexture));

		// create view of depth stencil texture
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = dsTexDesc.Format;
		dsvDesc.ViewDimension = (m_desc.samples > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;	
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateDepthStencilView(m_dsTexture.Get(), &dsvDesc, &m_dsv));
	}

}