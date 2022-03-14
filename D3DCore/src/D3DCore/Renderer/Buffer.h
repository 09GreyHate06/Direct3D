#pragma once
#include "D3DCore/Core/D3DCWindows.h"
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
		const void* data = nullptr; 
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

		static std::shared_ptr<VertexBuffer> Create(const VertexBufferDesc& desc);


	private:
		VertexBuffer(const VertexBufferDesc& desc);

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		uint32_t m_stride = 0;
		uint32_t m_size = 0;
	};




	struct IndexBufferDesc
	{
		const uint32_t* data = nullptr;
		uint32_t size = 0;
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
		
		static std::shared_ptr<IndexBuffer> Create(const IndexBufferDesc& desc);

	private:
		IndexBuffer(const IndexBufferDesc& desc);

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
		uint32_t m_size;
		uint32_t m_count;
	};
}