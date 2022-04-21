#include "GlobalAsset.h"

std::unordered_map<std::string, std::shared_ptr<d3dcore::Shader>> GlobalAsset::s_shaderMap;
std::unordered_map<std::string, std::shared_ptr<d3dcore::Texture2D>> GlobalAsset::s_textureMap;
std::unordered_map<std::string, std::shared_ptr<d3dcore::ConstantBuffer>> GlobalAsset::s_cbufMap;
std::unordered_map<std::string, std::shared_ptr<d3dcore::VertexBuffer>> GlobalAsset::s_vbufMap;
std::unordered_map<std::string, std::shared_ptr<d3dcore::IndexBuffer>> GlobalAsset::s_ibufMap;

std::shared_ptr<d3dcore::Shader> GlobalAsset::CreateAndAddShader(const std::string& key, const std::string& vsFilename, const std::string& psFilename)
{
	D3DC_ASSERT(s_shaderMap.find(key) == s_shaderMap.end(), "Storing duplicate!");
	auto shader = d3dcore::Shader::Create(vsFilename, psFilename);
	s_shaderMap[key] = shader;
	return std::move(shader);
}

std::shared_ptr<d3dcore::Shader> GlobalAsset::CreateAndAddShader(const std::string& key, const std::string& vsFilename)
{
	D3DC_ASSERT(s_shaderMap.find(key) == s_shaderMap.end(), "Storing duplicate!");

	auto shader = d3dcore::Shader::Create(vsFilename);
	s_shaderMap[key] = shader;
	return shader;
}

std::shared_ptr<d3dcore::Shader> GlobalAsset::AddShader(const std::string& key, const std::shared_ptr<d3dcore::Shader>& shader)
{
	s_shaderMap[key] = shader;
	return shader;
}

std::shared_ptr<d3dcore::Shader> GlobalAsset::GetShader(const std::string& key)
{
	D3DC_ASSERT(s_shaderMap.find(key) != s_shaderMap.end(), "Shader does not exist!");
	return s_shaderMap[key];
}

void GlobalAsset::RemoveShader(const std::string& key)
{
	D3DC_ASSERT(s_shaderMap.find(key) != s_shaderMap.end(), "Shader does not exist!");
	s_shaderMap.erase(key);
}

std::shared_ptr<d3dcore::Texture2D> GlobalAsset::CreateAndAddTexture(const std::string& key, const std::string& filename, const bool flipImageY, const d3dcore::Texture2DDesc& desc)
{
	D3DC_ASSERT(s_textureMap.find(key) == s_textureMap.end(), "Storing duplicate!");
	auto texture = d3dcore::Texture2D::Create(filename, flipImageY, desc);
	s_textureMap[key] = texture;
	return texture;
}

std::shared_ptr<d3dcore::Texture2D> GlobalAsset::CreateAndAddTexture(const std::string& key, const void* pixels, const d3dcore::Texture2DDesc& desc)
{
	D3DC_ASSERT(s_textureMap.find(key) == s_textureMap.end(), "Storing duplicate!");
	auto texture = d3dcore::Texture2D::Create(pixels, desc);
	s_textureMap[key] = texture;
	return texture;
}

std::shared_ptr<d3dcore::Texture2D> GlobalAsset::AddTexture(const std::string& key, const std::shared_ptr<d3dcore::Texture2D>& texture)
{
	D3DC_ASSERT(s_textureMap.find(key) == s_textureMap.end(), "Storing duplicate!");
	s_textureMap[key] = texture;
	return texture;
}

std::shared_ptr<d3dcore::Texture2D> GlobalAsset::GetTexture(const std::string& key)
{
	D3DC_ASSERT(s_textureMap.find(key) != s_textureMap.end(), "Texture does not exist!");
	return s_textureMap[key];
}

void GlobalAsset::RemoveTexture(const std::string& key)
{
	D3DC_ASSERT(s_textureMap.find(key) != s_textureMap.end(), "Texture does not exist!");
	s_textureMap.erase(key);
}

std::shared_ptr<d3dcore::ConstantBuffer> GlobalAsset::CreateAndAddCBuf(const std::string& key, const void* data, uint32_t size)
{
	D3DC_ASSERT(s_cbufMap.find(key) == s_cbufMap.end(), "Storing duplicate!");
	auto cbuf = d3dcore::ConstantBuffer::Create(data, size);
	s_cbufMap[key] = cbuf;
	return cbuf;
}

std::shared_ptr<d3dcore::ConstantBuffer> GlobalAsset::CreateAndAddCBuf(const std::string& key, uint32_t size)
{
	D3DC_ASSERT(s_cbufMap.find(key) == s_cbufMap.end(), "Storing duplicate!");
	auto cbuf = d3dcore::ConstantBuffer::Create(size);
	s_cbufMap[key] = cbuf;
	return cbuf;
}

std::shared_ptr<d3dcore::ConstantBuffer> GlobalAsset::AddCBuf(const std::string& key, const std::shared_ptr<d3dcore::ConstantBuffer>& cbuf)
{
	D3DC_ASSERT(s_cbufMap.find(key) == s_cbufMap.end(), "Storing duplicate!");
	s_cbufMap[key] = cbuf;
	return cbuf;
}

std::shared_ptr<d3dcore::ConstantBuffer> GlobalAsset::GetCBuf(const std::string& key)
{
	D3DC_ASSERT(s_cbufMap.find(key) != s_cbufMap.end(), "Texture does not exist!");
	return s_cbufMap[key];
}

void GlobalAsset::RemoveCBuf(const std::string& key)
{
	D3DC_ASSERT(s_cbufMap.find(key) != s_cbufMap.end(), "Texture does not exist!");
	s_cbufMap.erase(key);
}

std::shared_ptr<d3dcore::VertexBuffer> GlobalAsset::CreateAndAddVertexBuffer(const std::string& key, const void* data, const d3dcore::VertexBufferDesc& desc)
{
	D3DC_ASSERT(s_vbufMap.find(key) == s_vbufMap.end(), "Storing duplicate");
	auto vbuf = d3dcore::VertexBuffer::Create(data, desc);
	s_vbufMap[key] = vbuf;
	return vbuf;
}

std::shared_ptr<d3dcore::VertexBuffer> GlobalAsset::AddVertexBuffer(const std::string& key, const std::shared_ptr<d3dcore::VertexBuffer>& vbuf)
{
	D3DC_ASSERT(s_vbufMap.find(key) == s_vbufMap.end(), "Storing duplicate!");
	s_vbufMap[key] = vbuf;
	return vbuf;
}

std::shared_ptr<d3dcore::VertexBuffer> GlobalAsset::GetVertexBuffer(const std::string& key)
{
	D3DC_ASSERT(s_vbufMap.find(key) != s_vbufMap.end(), "Vertex Buffer does not exist");
	return s_vbufMap[key];
}

void GlobalAsset::RemoveVertexBuffer(const std::string& key)
{
	D3DC_ASSERT(s_vbufMap.find(key) != s_vbufMap.end(), "Vertex Buffer does not exist");
	s_vbufMap.erase(key);
}

std::shared_ptr<d3dcore::IndexBuffer> GlobalAsset::CreateAndAddIndexBuffer(const std::string& key, const uint32_t* data, const d3dcore::IndexBufferDesc& desc)
{
	D3DC_ASSERT(s_ibufMap.find(key) == s_ibufMap.end(), "Storing duplicate!");
	auto ibuf = d3dcore::IndexBuffer::Create(data, desc);
	s_ibufMap[key] = ibuf;
	return ibuf;
}

std::shared_ptr<d3dcore::IndexBuffer> GlobalAsset::AddIndexBuffer(const std::string& key, const std::shared_ptr<d3dcore::IndexBuffer>& ibuf)
{
	D3DC_ASSERT(s_ibufMap.find(key) == s_ibufMap.end(), "Storing duplicate!");
	s_ibufMap[key] = ibuf;
	return ibuf;
}

std::shared_ptr<d3dcore::IndexBuffer> GlobalAsset::GetIndexBuffer(const std::string& key)
{
	D3DC_ASSERT(s_ibufMap.find(key) != s_ibufMap.end(), "Index Buffer does not exist");
	return s_ibufMap[key];
}

void GlobalAsset::RemoveIndexBuffer(const std::string& key)
{
	D3DC_ASSERT(s_ibufMap.find(key) != s_ibufMap.end(), "Index Buffer does not exist");
	s_ibufMap.erase(key);
}
