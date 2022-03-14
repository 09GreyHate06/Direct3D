#include "d3dcpch.h"
#include "Buffer.h"
#include "D3DContext.h"

namespace d3dcore
{
	ConstantBuffer::ConstantBuffer(const void* data, uint32_t size)
		: m_size(size)
	{
		HRESULT hr;

		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = size;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA srd = {};
		srd.pSysMem = data;
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateBuffer(&desc, &srd, &m_constantBuffer));
	}

	ConstantBuffer::ConstantBuffer(uint32_t size)
		: m_size(size)
	{
		HRESULT hr;

		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = size;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateBuffer(&desc, nullptr, &m_constantBuffer));
	}

	void ConstantBuffer::SetData(const void* data)
	{
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE msr = {};
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDeviceContext()->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
		memcpy(msr.pData, data, m_size);
		D3DContext::GetDeviceContext()->Unmap(m_constantBuffer.Get(), 0);
	}

	void ConstantBuffer::VSBind(uint32_t slot) const
	{
		D3DContext::GetDeviceContext()->VSSetConstantBuffers(slot, 1, m_constantBuffer.GetAddressOf());
	}

	void ConstantBuffer::PSBind(uint32_t slot) const
	{
		D3DContext::GetDeviceContext()->PSSetConstantBuffers(slot, 1, m_constantBuffer.GetAddressOf());
	}

	std::shared_ptr<ConstantBuffer> ConstantBuffer::Create(const void* data, uint32_t size)
	{
		return std::shared_ptr<ConstantBuffer>(new ConstantBuffer(data, size));
	}

	std::shared_ptr<ConstantBuffer> ConstantBuffer::Create(uint32_t size)
	{
		return std::shared_ptr<ConstantBuffer>(new ConstantBuffer(size));
	}











	VertexBuffer::VertexBuffer(const VertexBufferDesc& desc)
		: m_size(desc.size), m_stride(desc.stride)
	{
		HRESULT hr;

		D3D11_BUFFER_DESC vDesc = {};
		vDesc.ByteWidth = m_size;
		vDesc.Usage = desc.usage;
		vDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vDesc.CPUAccessFlags = desc.cpuAccessFlag;
		vDesc.MiscFlags = 0;
		vDesc.StructureByteStride = m_stride;

		if (desc.data)
		{
			D3D11_SUBRESOURCE_DATA srd = {};
			srd.pSysMem = desc.data;
			D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateBuffer(&vDesc, &srd, &m_vertexBuffer));
		}
		else
		{
			D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateBuffer(&vDesc, nullptr, &m_vertexBuffer));
		}
	}

	void VertexBuffer::Bind() const
	{
		const uint32_t offset = 0;
		D3DContext::GetDeviceContext()->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &m_stride, &offset);
	}

	std::shared_ptr<VertexBuffer> VertexBuffer::Create(const VertexBufferDesc& desc)
	{
		return std::shared_ptr<VertexBuffer>(new VertexBuffer(desc));
	}

	void VertexBuffer::SetData(const void* data)
	{
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE msr = {};
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDeviceContext()->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
		memcpy(msr.pData, data, m_size);
		D3DContext::GetDeviceContext()->Unmap(m_vertexBuffer.Get(), 0);
	}





	IndexBuffer::IndexBuffer(const IndexBufferDesc& desc)
		: m_size(desc.size), m_count(desc.size / sizeof(uint32_t))
	{
		HRESULT hr;

		D3D11_BUFFER_DESC iDesc = {};
		iDesc.ByteWidth = m_size;
		iDesc.Usage = desc.usage;
		iDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		iDesc.CPUAccessFlags = desc.cpuAccessFlag;
		iDesc.MiscFlags = 0;
		iDesc.StructureByteStride = sizeof(uint32_t);

		if (desc.data)
		{
			D3D11_SUBRESOURCE_DATA srd = {};
			srd.pSysMem = desc.data;
			D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateBuffer(&iDesc, &srd, &m_indexBuffer));
		}
		else
		{
			D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateBuffer(&iDesc, nullptr, &m_indexBuffer));
		}
	}

	std::shared_ptr<IndexBuffer> IndexBuffer::Create(const IndexBufferDesc& desc)
	{
		return std::shared_ptr<IndexBuffer>(new IndexBuffer(desc));
	}

	void IndexBuffer::Bind() const
	{
		D3DContext::GetDeviceContext()->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	}

	void IndexBuffer::SetData(const uint32_t* data)
	{
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE msr = {};
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDeviceContext()->Map(m_indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
		memcpy(msr.pData, data, m_size);
		D3DContext::GetDeviceContext()->Unmap(m_indexBuffer.Get(), 0);
	}
}