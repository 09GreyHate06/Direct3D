#pragma once
#include "D3DCore/Core/D3DCWindows.h"
#include "Shader.h"
#include <d3d11.h>
#include <wrl.h>
#include <filesystem>

namespace d3dcore
{
	struct Texture2DDesc
	{
		uint32_t width = 0;  // set to 0 to set the loaded image original width
		uint32_t height = 0; // set to 0 to set the loaded image original height
		D3D11_FILTER filter = D3D11_FILTER_ANISOTROPIC;
		D3D11_TEXTURE_ADDRESS_MODE addressU = D3D11_TEXTURE_ADDRESS_WRAP;
		D3D11_TEXTURE_ADDRESS_MODE addressV = D3D11_TEXTURE_ADDRESS_WRAP;
		float borderColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // if texture address is D3D11_TEXTURE_ADDRESS_BORDER
	};

	class Texture2D
	{
	public:
		~Texture2D() = default;
		//void Set(const std::string& filename, const bool flipImageY, const Texture2DDesc& desc);
		//void Set(const void* pixels, const Texture2DDesc& desc);

		const Texture2DDesc& GetDesc() const { return m_desc; }

		void VSBind(uint32_t slot = 0) const;
		void PSBind(uint32_t slot = 0) const;

		ID3D11ShaderResourceView* GetView() const { return m_textureView.Get(); }
		const std::filesystem::path& GetFilepath() const { return m_path; }
		std::string GetFilename() const { return m_path.filename().string(); }

		static std::shared_ptr<Texture2D> Create(const std::string& filename, const bool flipImageY, const Texture2DDesc& desc);
		static std::shared_ptr<Texture2D> Create(const void* pixels, const Texture2DDesc& desc);
		//static std::shared_ptr<Texture2D> CreateEmpty();

	private:
		Texture2D(const std::string& filename, const bool flipImageY, const Texture2DDesc& desc);
		Texture2D(const void* pixels, const Texture2DDesc& desc);

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureView;

		Texture2DDesc m_desc;

		std::filesystem::path m_path;
	};
}