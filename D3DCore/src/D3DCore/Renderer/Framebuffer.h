#pragma once
#include "D3DCore/Core/D3DCWindows.h"
#include <d3d11.h>
#include <wrl.h>

namespace d3dcore
{
	struct FramebufferDesc
	{
		uint32_t width = 0, height = 0;
		uint32_t samples = 1;
		uint32_t sampleQuality = 0;
		bool hasDepth = false;
	};

	class Framebuffer
	{
	public:
		~Framebuffer() = default;

		const FramebufferDesc& GetDesc() const { return m_desc; }
		void Resize(uint32_t width, uint32_t height);
		ID3D11ShaderResourceView* GetView() const { return m_renderTargetSrv.Get(); }
		void VSBindAsTexture2D(uint32_t slot = 0);
		void PSBindAsTexture2D(uint32_t slot = 0);

		//static void Blit(Framebuffer* dst, Framebuffer* src);
		// src = non MSAA; dst = MSAA
		static void Resolve(Framebuffer* dst, Framebuffer* src);
		static std::shared_ptr<Framebuffer> Create(const FramebufferDesc& desc);
	private:
		Framebuffer(const FramebufferDesc& desc);

		FramebufferDesc m_desc;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_renderTargetTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_renderTargetSrv;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilTexture;

	private:
		friend class Renderer;
	};
}