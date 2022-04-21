#pragma once
#include "D3DCore/Core/D3DCWindows.h"
#include "D3DCore/Core/D3DCException.h"

#include <wrl.h>
#include <d3d11.h>
#include <d3dcompiler.h>

namespace d3dcore
{
	class Shader
	{
	public:
		~Shader() = default;
		void Bind();

		uint32_t GetVSResBinding(const std::string& name);
		uint32_t GetPSResBinding(const std::string& name);

		static std::shared_ptr<Shader> Create(const std::string& vertexFilename, const std::string& pixelFilename);
		static std::shared_ptr<Shader> Create(const std::string& vertexFilename);

	private:
		Shader(const std::string& vertexFilename, const std::string& pixelFilename);
		Shader(const std::string& vertexFilename);

		void CreateInputLayout();

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vShader = nullptr;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pShader = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> m_vsByteCode = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> m_psByteCode = nullptr;

		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_vsInputLayout = nullptr;

		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> m_vsReflection = nullptr;
		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> m_psReflection = nullptr;

		std::unordered_map<std::string, uint32_t> m_resVSBindingCache;
		std::unordered_map<std::string, uint32_t> m_resPSBindingCache;


		// exception
	public:
		class Exception : public D3DCException
		{
			using D3DCException::D3DCException;
		};

	public:
		class ShaderCompilationException : public Exception
		{
		public:
			ShaderCompilationException(int line, const std::string& file, HRESULT hr, const std::string& info);
			virtual const char* what() const override;
			virtual const char* GetType() const override { return "Shader Compilation Exception"; }
			const std::string& GetErrorInfo() const { return m_info; }
			HRESULT GetErrorCode() const { return m_hr; }

		private:
			std::string m_info;
			HRESULT m_hr;
		};
	};
}