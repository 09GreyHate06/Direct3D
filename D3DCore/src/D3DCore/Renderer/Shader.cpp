#include "d3dcpch.h"
#include "Shader.h"
#include "D3DContext.h"
#include "D3DCore/Utils/Loader.h"

using namespace Microsoft::WRL;

#define D3DC_SHADER_COMPILATION_EXCEPT(hr, msg) d3dcore::Shader::ShaderCompilationException::ShaderCompilationException(__LINE__, __FILE__, (hr), (msg))

namespace d3dcore
{
	Shader::Shader(const std::string& vertexFilename, const std::string& pixelFilename)
	{
		HRESULT hr;

		std::string vsSource = utils::LoadTextFile(vertexFilename);
		std::string psSource = utils::LoadTextFile(pixelFilename);
		
		ComPtr<ID3DBlob> errorBlob;
		if (FAILED(hr = D3DCompile(vsSource.data(), vsSource.size(), nullptr, nullptr, nullptr, "main", "vs_4_0", 0, 0, &m_vsByteCode, &errorBlob)))
			throw D3DC_SHADER_COMPILATION_EXCEPT(hr, static_cast<const char*>(errorBlob->GetBufferPointer()));
		if (FAILED(hr = D3DCompile(psSource.data(), psSource.size(), nullptr, nullptr, nullptr, "main", "ps_4_0", 0, 0, &m_psByteCode, &errorBlob)))
			throw D3DC_SHADER_COMPILATION_EXCEPT(hr, static_cast<const char*>(errorBlob->GetBufferPointer()));

		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateVertexShader(m_vsByteCode->GetBufferPointer(), m_vsByteCode->GetBufferSize(), nullptr, &m_vShader));
		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreatePixelShader(m_psByteCode->GetBufferPointer(), m_psByteCode->GetBufferSize(), nullptr, &m_pShader));

		D3DC_CONTEXT_THROW_INFO(D3DReflect(m_vsByteCode->GetBufferPointer(), m_vsByteCode->GetBufferSize(), __uuidof(ID3D11ShaderReflection), &m_vsReflection));
		D3DC_CONTEXT_THROW_INFO(D3DReflect(m_psByteCode->GetBufferPointer(), m_psByteCode->GetBufferSize(), __uuidof(ID3D11ShaderReflection), &m_psReflection));

		CreateInputLayout();
	}

	std::shared_ptr<Shader> Shader::Create(const std::string& vertexFilename, const std::string& pixelFilename)
	{
		return std::shared_ptr<Shader>(new Shader(vertexFilename, pixelFilename));
	}

	void Shader::Bind()
	{
		D3DContext::GetDeviceContext()->IASetInputLayout(m_vsInputLayout.Get());
		D3DContext::GetDeviceContext()->VSSetShader(m_vShader.Get(), nullptr, 0);
		D3DContext::GetDeviceContext()->PSSetShader(m_pShader.Get(), nullptr, 0);
	}

	uint32_t Shader::GetVSResBinding(const std::string& name)
	{
		if (m_resVSBindingCache.find(name) != m_resVSBindingCache.end())
			return m_resVSBindingCache[name];

		HRESULT hr;
		D3D11_SHADER_INPUT_BIND_DESC desc = {};
		D3DC_CONTEXT_THROW_INFO(m_vsReflection->GetResourceBindingDescByName(name.c_str(), &desc));
		uint32_t slot = desc.BindPoint;
		m_resVSBindingCache[name] = slot;
		return slot;
	}

	uint32_t Shader::GetPSResBinding(const std::string& name)
	{
		if (m_resPSBindingCache.find(name) != m_resPSBindingCache.end())
			return m_resPSBindingCache[name];

		HRESULT hr;
		D3D11_SHADER_INPUT_BIND_DESC desc = {};
		D3DC_CONTEXT_THROW_INFO(m_psReflection->GetResourceBindingDescByName(name.c_str(), &desc));
		uint32_t slot = desc.BindPoint;
		m_resPSBindingCache[name] = slot;
		return slot;
	}

	void Shader::CreateInputLayout()
	{
		HRESULT hr;

		D3D11_SHADER_DESC shaderDesc = {};
		D3DC_CONTEXT_THROW_INFO(m_vsReflection->GetDesc(&shaderDesc));

		// Read input layout description from shader info
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
		for (uint32_t i = 0; i < shaderDesc.InputParameters; i++)
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc = {};
			D3DC_CONTEXT_THROW_INFO(m_vsReflection->GetInputParameterDesc(i, &paramDesc));

			// fill out input element desc
			D3D11_INPUT_ELEMENT_DESC elementDesc = {};
			elementDesc.SemanticName = paramDesc.SemanticName;
			elementDesc.SemanticIndex = paramDesc.SemanticIndex;
			elementDesc.InputSlot = 0;
			elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elementDesc.InstanceDataStepRate = 0;

			// determine DXGI format
			if (paramDesc.Mask == 1)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			}
			else if (paramDesc.Mask <= 3)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			}
			else if (paramDesc.Mask <= 7)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (paramDesc.Mask <= 15)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}

			inputLayoutDesc.push_back(elementDesc);
		}

		D3DC_CONTEXT_THROW_INFO(D3DContext::GetDevice()->CreateInputLayout(inputLayoutDesc.data(), static_cast<uint32_t>(inputLayoutDesc.size()), 
			m_vsByteCode->GetBufferPointer(), m_vsByteCode->GetBufferSize(), &m_vsInputLayout));
	}



	Shader::ShaderCompilationException::ShaderCompilationException(int line, const std::string& file, HRESULT hr, const std::string& info)
		: Exception(line, file), m_hr(hr), m_info(info)
	{
	}

	const char* Shader::ShaderCompilationException::what() const
	{
		std::ostringstream oss;
		oss << GetType() << '\n'
			<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
			<< std::dec << " (" << static_cast<uint32_t>(GetErrorCode()) << ") " << '\n'
			<< "[Error Info]: " << GetErrorInfo() << '\n';
		oss << GetOriginString();

		m_whatBuffer = oss.str();
		return m_whatBuffer.c_str();
	}
}