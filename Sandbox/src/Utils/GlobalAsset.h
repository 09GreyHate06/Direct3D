#pragma once
#include <D3DCore.h>

class GlobalAsset
{
public:
	static std::shared_ptr<d3dcore::Shader> CreateAndAddShader(const std::string& key, const std::string& vsFilename, const std::string& psFilename);
	static std::shared_ptr<d3dcore::Shader> CreateAndAddShader(const std::string& key, const std::string& vsFilename);
	static std::shared_ptr<d3dcore::Shader> AddShader(const std::string& key, const std::shared_ptr<d3dcore::Shader>& shader);
	static std::shared_ptr<d3dcore::Shader> GetShader(const std::string& key);
	static void RemoveShader(const std::string& key);

	static std::shared_ptr<d3dcore::Texture2D> CreateAndAddTexture(const std::string& key, const std::string& filename, const bool flipImageY, const d3dcore::Texture2DDesc& desc);
	static std::shared_ptr<d3dcore::Texture2D> CreateAndAddTexture(const std::string& key, const void* pixels, const d3dcore::Texture2DDesc& desc);
	static std::shared_ptr<d3dcore::Texture2D> AddTexture(const std::string& key, const std::shared_ptr<d3dcore::Texture2D>& texture);
	static std::shared_ptr<d3dcore::Texture2D> GetTexture(const std::string& key);
	static void RemoveTexture(const std::string& key);

	static std::shared_ptr<d3dcore::ConstantBuffer> CreateAndAddCBuf(const std::string& key, const void* data, uint32_t size);
	static std::shared_ptr<d3dcore::ConstantBuffer> CreateAndAddCBuf(const std::string& key, uint32_t size);
	static std::shared_ptr<d3dcore::ConstantBuffer> AddCBuf(const std::string& key, const std::shared_ptr<d3dcore::ConstantBuffer>& cbuf);
	static std::shared_ptr<d3dcore::ConstantBuffer> GetCBuf(const std::string& key);
	static void RemoveCBuf(const std::string& key);

	static std::shared_ptr<d3dcore::VertexBuffer> CreateAndAddVertexBuffer(const std::string& key, const void* data, const d3dcore::VertexBufferDesc& desc);
	static std::shared_ptr<d3dcore::VertexBuffer> AddVertexBuffer(const std::string& key, const std::shared_ptr<d3dcore::VertexBuffer>& vbuf);
	static std::shared_ptr<d3dcore::VertexBuffer> GetVertexBuffer(const std::string& key);
	static void RemoveVertexBuffer(const std::string& key);

	static std::shared_ptr<d3dcore::IndexBuffer> CreateAndAddIndexBuffer(const std::string& key, const uint32_t* data, const d3dcore::IndexBufferDesc& desc);
	static std::shared_ptr<d3dcore::IndexBuffer> AddIndexBuffer(const std::string& key, const std::shared_ptr<d3dcore::IndexBuffer>& ibuf);
	static std::shared_ptr<d3dcore::IndexBuffer> GetIndexBuffer(const std::string& key);
	static void RemoveIndexBuffer(const std::string& key);
private:
	static std::unordered_map<std::string, std::shared_ptr<d3dcore::Shader>> s_shaderMap;
	static std::unordered_map<std::string, std::shared_ptr<d3dcore::Texture2D>> s_textureMap;
	static std::unordered_map<std::string, std::shared_ptr<d3dcore::ConstantBuffer>> s_cbufMap;
	static std::unordered_map<std::string, std::shared_ptr<d3dcore::VertexBuffer>> s_vbufMap;
	static std::unordered_map<std::string, std::shared_ptr<d3dcore::IndexBuffer>> s_ibufMap;
};