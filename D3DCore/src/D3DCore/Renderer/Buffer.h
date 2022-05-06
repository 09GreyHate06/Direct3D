#pragma once
#include "D3DCore/Core/D3DCWindows.h"
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>

namespace d3dcore
{
	class ConstantBuffer
	{
	public:
		~ConstantBuffer() = default;

		void SetData(const void* data);

		void VSBind(uint32_t slot = 0) const;
		void PSBind(uint32_t slot = 0) const;

#ifdef D3DC_INTERNALS
		const ID3D11Buffer* GetNativeBuffer() const { return m_constantBuffer.Get(); }
		ID3D11Buffer* GetNativeBuffer() { return m_constantBuffer.Get(); }
#endif

		static std::shared_ptr<ConstantBuffer> Create(const void* data, uint32_t size);
		static std::shared_ptr<ConstantBuffer> Create(uint32_t size);

	private:
		ConstantBuffer(const void* data, uint32_t size);
		ConstantBuffer(uint32_t size);

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
		uint32_t m_size;
	};






	struct VertexBufferDesc
	{
		uint32_t size = 0; 
		uint32_t stride = 0;
		D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
		uint32_t cpuAccessFlag = 0;
	};

	class VertexBuffer
	{
	public:
		~VertexBuffer() = default;
		void Bind() const;
		void SetData(const void* data);

#ifdef D3DC_INTERNALS
		const ID3D11Buffer* GetNativeBuffer() const { return m_vertexBuffer.Get(); }
		ID3D11Buffer* GetNativeBuffer() { return m_vertexBuffer.Get(); }
#endif

		static std::shared_ptr<VertexBuffer> Create(const void* data, const VertexBufferDesc& desc);


	private:
		VertexBuffer(const void* data, const VertexBufferDesc& desc);

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		uint32_t m_stride = 0;
		uint32_t m_size = 0;
	};




	struct IndexBufferDesc
	{
		uint32_t count = 0;
		D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
		uint32_t cpuAccessFlag = 0;
	};

	class IndexBuffer
	{
	public:
		~IndexBuffer() = default;
		void Bind() const;
		void SetData(const uint32_t* data);
		uint32_t GetCount() const { return m_count; }

#ifdef D3DC_INTERNALS
		const ID3D11Buffer* GetNativeBuffer() const { return m_indexBuffer.Get(); }
		ID3D11Buffer* GetNativeBuffer() { return m_indexBuffer.Get(); }
#endif

		static std::shared_ptr<IndexBuffer> Create(const uint32_t* data, const IndexBufferDesc& desc);

	private:
		IndexBuffer(const uint32_t* data, const IndexBufferDesc& desc);

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
		uint32_t m_size;
		uint32_t m_count;
	};
}