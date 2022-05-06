#include "d3dcpch.h"
#include "Texture.h"
#include "D3DContext.h"
#include "D3DCore/Utils/Loader.h"

namespace d3dcore
{
	static constexpr uint32_t s_reqComponents = 4;

	Texture2D::Texture2D(const std::string& filename, const bool flipImageY, const Texture2DDesc& desc)
		: m_desc(desc)
	{
		utils::ImageData imageData = utils::LoadImageFile(filename, flipImageY, s_reqComponents);
		
		uint32_t width = desc.width ? desc.width : imageData.width;
		uint32_t height = desc.height ? desc.height : imageData.height;

		Texture2DDesc descB = desc;
		descB.width = width;
		descB.height = height;

		Init(imageData.pixels, descB);

		utils::FreeImageData(imageData);
	}

	Texture2D::Texture2D(const void* pixels, const Texture2DDesc& desc)
		: m_desc(desc)
	{
		Init(pixels, desc);
	}

	void Texture2D::Init(const void* pixels, const Texture2DDesc& desc)
	{
		HRESULT hr;

		// create texture resource
		D3D11_TEXTURE2D_DESC t2dDesc = {};
		t2dDesc.Width = desc.width;
		t2dDesc.Height = desc.height;
		t2dDesc.MipLevels = 0; // 0 to generate a full set of subtextures (1 for a multisampled texture)
		t2dDesc.ArraySize = 1;
		t2dDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		t2dDesc.SampleDesc.Count = 1;
		t2dDesc.SampleDesc.Quality = 0;
		t2dDesc.Usage = D3D11_USAGE_DEFAULT;
		t2dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		t2dDesc.CPUAccessFlags = 0;
		t2dDesc.MiscFlags = desc.genMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateTexture2D(&t2dDesc, nullptr, &m_texture));

		// write image data into top mip level
		D3DContext::GetDeviceContext()->UpdateSubresource(m_texture.Get(), 0, nullptr, pixels, s_reqComponents * desc.width, 0);

		// create the resource view on the texture
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = t2dDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = desc.genMips ? -1 : 1;
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_textureView));

		// generate the mip chain using the gpu rendering pipeline
		D3DContext::GetDeviceContext()->GenerateMips(m_textureView.Get());

		SetSamplerState(desc.sampler);
	}

	void Texture2D::SetSamplerState(const SamplerState& sampler)
	{
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
		samplerDesc.Filter = sampler.filter;
		samplerDesc.AddressU = sampler.addressU;
		samplerDesc.AddressV = sampler.addressV;
		samplerDesc.AddressW = sampler.addressV;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		memcpy(samplerDesc.BorderColor, sampler.borderColor, 4 * sizeof(float));
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		HRESULT hr;
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

