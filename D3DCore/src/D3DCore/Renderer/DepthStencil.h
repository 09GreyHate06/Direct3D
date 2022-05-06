#pragma once

#include <d3d11.h>
#include <wrl.h>

namespace d3dcore
{
	struct DepthStencilDesc
	{
		uint32_t width = 0, height = 0;
		uint32_t samples = 1;
		uint32_t sampleQuality = 0;
	};

	class DepthStencil
	{
	public:
		~DepthStencil() = default;

		void Clear(uint32_t clearFlags, float depth, uint8_t stencil);

		const DepthStencilDesc& GetDesc() const { return m_desc; }
		void Resize(uint32_t width, uint32_t height);

#ifdef D3DC_INTERNALS
		const ID3D11Texture2D* GetDSTexture2D() const { return m_dsTexture.Get(); }
		ID3D11Texture2D* GetDSTexture2D() { return m_dsTexture.Get(); }
		const ID3D11DepthStencilView* GetDSV() const { return m_dsv.Get(); }
		ID3D11DepthStencilView* GetDSV() { return m_dsv.Get(); }
#endif // D3DC_INTERNALS

		static std::shared_ptr<DepthStencil> Create(const DepthStencilDesc& desc);

	private:
		DepthStencil(const DepthStencilDesc& desc);

		DepthStencilDesc m_desc;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_dsTexture;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;

		friend class Renderer;
	};
}