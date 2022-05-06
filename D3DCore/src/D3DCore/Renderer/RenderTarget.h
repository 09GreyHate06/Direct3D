#pragma once
#include "D3DCore/Core/D3DCWindows.h"
#include <d3d11.h>
#include <wrl.h>
#include "Texture.h"
#include "DepthStencil.h"

namespace d3dcore
{
	struct RenderTargetDesc
	{
		uint32_t width = 0, height = 0;
		uint32_t samples = 1;
		uint32_t sampleQuality = 0;
	};

	class RenderTarget
	{
	public:
		~RenderTarget() = default;

		void Clear(float r, float g, float b, float a);
		const RenderTargetDesc& GetDesc() const { return m_desc; }
		void SetTexture2DSamplerState(const SamplerState& sampler);
		void Resize(uint32_t width, uint32_t height);
		ID3D11ShaderResourceView* GetView() const { return m_rtSrv.Get(); }
		void VSBindAsTexture2D(uint32_t slot = 0) const;
		void PSBindAsTexture2D(uint32_t slot = 0) const;

#ifdef D3DC_INTERNALS
		void Bind(const DepthStencil* ds);
		const ID3D11RenderTargetView* GetRTV() const { return m_rtv.Get(); }
		ID3D11RenderTargetView* GetRTV() { return m_rtv.Get(); }
		const ID3D11Texture2D* GetRTTexture2D() const { return m_rtTexture.Get(); }
		ID3D11Texture2D* GetRTTexture2D() { return m_rtTexture.Get(); }
#endif // D3DC_INTERNALS


		//static void Blit(Framebuffer* dst, Framebuffer* src);
		// dst = non MSAA; src = MSAA
		static void Resolve(RenderTarget* dst, RenderTarget* src);
		static std::shared_ptr<RenderTarget> Create(const RenderTargetDesc& desc);

	private:
		RenderTarget(const RenderTargetDesc& desc);

		RenderTargetDesc m_desc;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_rtTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_rtSrv;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_texSampler;

		friend class Renderer;
	};
}
