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

#ifdef D3DC_INTERNALS
		const ID3D11VertexShader* GetNativeVertexShader() const { return m_vShader.Get(); }
		ID3D11VertexShader* GetNativeVertexShader() { return m_vShader.Get(); }

		const ID3D11PixelShader* GetNativePixelShader() const { return m_pShader.Get(); }
		ID3D11PixelShader* GetNativePixelShader() { return m_pShader.Get(); }

		const ID3DBlob* GetVertexByteCode() const { return m_vsByteCode.Get(); }
		ID3DBlob* GetVertexByteCode() { return m_vsByteCode.Get(); }

		const ID3DBlob* GetPixelByteCode() const { return m_psByteCode.Get(); }
		ID3DBlob* GetPixelByteCode() { return m_psByteCode.Get(); }

		const ID3D11InputLayout* GetVertexInputLayout() const { return m_vsInputLayout.Get(); }
		ID3D11InputLayout* GetVertexInputLayout() { return m_vsInputLayout.Get(); }

		const ID3D11ShaderReflection* GetVertexReflection() const { return m_vsReflection.Get(); }
		ID3D11ShaderReflection* GetVertexReflection() { return m_vsReflection.Get(); }

		const ID3D11ShaderReflection* GetPixelReflection() const { return m_psReflection.Get(); }
		ID3D11ShaderReflection* GetPixelReflection() { return m_psReflection.Get(); }
#endif

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