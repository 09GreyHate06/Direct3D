#include "d3dcpch.h"
#include "Texture.h"
#include "D3DContext.h"
#include "D3DCore/Utils/Loader.h"

namespace d3dcore
{
	Texture2D::Texture2D(const std::string& filename, const bool flipImageY, const Texture2DDesc& desc)
		: m_path(filename)
	{
		HRESULT hr;
		utils::ImageData imageData = utils::LoadImageFile(filename, flipImageY, 4);
		
		uint32_t width = desc.width ? desc.width : imageData.width;
		uint32_t height = desc.height ? desc.height : imageData.height;

		// create texture resource
		D3D11_TEXTURE2D_DESC t2dDesc = {};
		t2dDesc.Width = width;
		t2dDesc.Height = height;
		t2dDesc.MipLevels = 1;
		t2dDesc.ArraySize = 1;
		t2dDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		t2dDesc.SampleDesc.Count = 1;
		t2dDesc.SampleDesc.Quality = 0;
		t2dDesc.Usage = D3D11_USAGE_DEFAULT;
		t2dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		t2dDesc.CPUAccessFlags = 0;
		t2dDesc.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA srd = {};
		srd.pSysMem = imageData.pixels;
		srd.SysMemPitch = 4 * width;
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateTexture2D(&t2dDesc, &srd, &m_texture));

		// create the resource view on the texture
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = t2dDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_textureView));

		utils::FreeImageData(imageData);

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = desc.filter;
		samplerDesc.AddressU = desc.addressU;
		samplerDesc.AddressV = desc.addressV;
		samplerDesc.AddressW = desc.addressV;

		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateSamplerState(&samplerDesc, &m_samplerState));
	}

	Texture2D::Texture2D(const void* pixels, const Texture2DDesc& desc)
	{
		m_path = std::filesystem::path();

		HRESULT hr;

		// create texture resource
		D3D11_TEXTURE2D_DESC t2dDesc = {};
		t2dDesc.Width = desc.width;
		t2dDesc.Height = desc.height;
		t2dDesc.MipLevels = 1;
		t2dDesc.ArraySize = 1;
		t2dDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		t2dDesc.SampleDesc.Count = 1;
		t2dDesc.SampleDesc.Quality = 0;
		t2dDesc.Usage = D3D11_USAGE_DEFAULT;
		t2dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		t2dDesc.CPUAccessFlags = 0;
		t2dDesc.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA srd = {};
		srd.pSysMem = pixels;
		srd.SysMemPitch = 4 * desc.width;
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateTexture2D(&t2dDesc, &srd, &m_texture));

		// create the resource view on the texture
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = t2dDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_textureView));

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = desc.filter;
		samplerDesc.AddressU = desc.addressU;
		samplerDesc.AddressV = desc.addressV;
		samplerDesc.AddressW = desc.addressV;

		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateSamplerState(&samplerDesc, &m_samplerState));
	}

	void Texture2D::VSBind(uint32_t slot) const
	{
		D3DContext::GetDeviceContext()->VSSetSamplers(slot, 1, m_samplerState.GetAddressOf());
		D3DContext::GetDeviceContext()->VSSetShaderResources(slot, 1, m_textureView.GetAddressOf());
	}

	void Texture2D::PSBind(uint32_t slot) const
	{
		D3DContext::GetDeviceContext()->PSSetSamplers(slot, 1, m_samplerState.GetAddressOf());
		D3DContext::GetDeviceContext()->PSSetShaderResources(slot, 1, m_textureView.GetAddressOf());
	}

	std::shared_ptr<Texture2D> Texture2D::Create(const std::string& filename, const bool flipImageY, const Texture2DDesc& desc)
	{
		return std::shared_ptr<Texture2D>(new Texture2D(filename, flipImageY, desc));
	}

	std::shared_ptr<Texture2D> Texture2D::Create(const void* pixels, const Texture2DDesc& desc)
	{
		return std::shared_ptr<Texture2D>(new Texture2D(pixels, desc));
	}
}
