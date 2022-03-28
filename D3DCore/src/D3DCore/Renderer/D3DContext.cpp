#include "d3dcpch.h"
#include "D3DContext.h"
#include "D3DCore/Core/Base.h"
#include "DXError/dxerr.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

namespace d3dcore
{
#ifdef D3DC_DEBUG
	DxgiInfoManager D3DContext::s_infoManager;
#endif // D3DC_DEBUG

	ID3D11Device* D3DContext::s_device = nullptr;
	ID3D11DeviceContext* D3DContext::s_deviceContext = nullptr;
	IDXGISwapChain* D3DContext::s_swapChain = nullptr;

	uint32_t D3DContext::s_sampleCount;
	uint32_t D3DContext::s_sampleQuality;

	void D3DContext::Init(HWND hWnd, uint32_t sampleCount, uint32_t sampleQuality)
	{
		D3DC_CORE_ASSERT(IsWindow(hWnd), "Window cannot be null");

		s_sampleCount = sampleCount;
		s_sampleQuality = sampleQuality;

		DXGI_SWAP_CHAIN_DESC scDesc = {};
		scDesc.BufferDesc.Width = 0;
		scDesc.BufferDesc.Height = 0;
		scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scDesc.BufferDesc.RefreshRate.Numerator = 0;
		scDesc.BufferDesc.RefreshRate.Denominator = 0;
		scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scDesc.SampleDesc.Count = s_sampleCount;
		scDesc.SampleDesc.Quality = s_sampleQuality;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.BufferCount = 1;
		scDesc.OutputWindow = hWnd;
		scDesc.Windowed = TRUE;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		scDesc.Flags = 0;

		HRESULT hr;

		UINT swapCreateFlags = 0;
#ifdef D3DC_DEBUG
		swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // D3DC_DEBUG

		// create device and front/back buffers, swap chain and rendering context
		D3DC_CONTEXT_THROW_INFO(D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			swapCreateFlags,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&scDesc,
			&s_swapChain,
			&s_device,
			nullptr,
			&s_deviceContext
		));

		UINT quality;
		s_device->CheckMultisampleQualityLevels(scDesc.BufferDesc.Format, 4, &quality);
		D3DC_CORE_LOG_INFO("Quality msaa: {0}", quality);
	}

	void D3DContext::Shutdown()
	{
		D3DC_COM_SAFE_RELEASE(s_device);
		D3DC_COM_SAFE_RELEASE(s_deviceContext);
		D3DC_COM_SAFE_RELEASE(s_swapChain);
	}




	// exceptions
	D3DContext::HrException::HrException(int line, const std::string& file, HRESULT hr, const std::vector<std::string>& infoMessages)
		: Exception(line, file), m_hr(hr)
	{
		// join all info messages with newlines into single string
		for (const auto& m : infoMessages)
		{
			m_info += m;
			m_info.push_back('\n');
		}
		// remove final newline if exists
		if (!m_info.empty())
		{
			m_info.pop_back();
		}
	}

	const char* D3DContext::HrException::what() const
	{
		std::ostringstream oss;
		oss << GetType() << '\n'
			<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
			<< std::dec << " (" << static_cast<uint32_t>(GetErrorCode()) << ") " << '\n'
			<< "[Error String] " << GetErrorString() << '\n'
			<< "[Description]" << GetErrorDescription() << '\n';
		if (!m_info.empty())
			oss << "\n[Error Info]\n" << GetErrorInfo() << "\n\n";
		oss << GetOriginString();

		m_whatBuffer = oss.str();
		return m_whatBuffer.c_str();
	}

	std::string D3DContext::HrException::GetErrorString() const
	{
		return DXGetErrorStringA(m_hr);
	}

	std::string D3DContext::HrException::GetErrorDescription() const
	{
		char buffer[512];
		DXGetErrorDescriptionA(m_hr, buffer, sizeof(buffer));
		return buffer;
	}

	D3DContext::InfoException::InfoException(int line, const std::string& file, const std::vector<std::string>& infoMessages)
		: Exception(line, file)
	{
		// join all info messages with newlines into single string
		for (const auto& m : infoMessages)
		{
			m_info += m;
			m_info.push_back('\n');
		}
		// remove final newline if exists
		if (!m_info.empty())
		{
			m_info.pop_back();
		}
	}

	const char* D3DContext::InfoException::what() const
	{
		std::ostringstream oss;
		oss << GetType() << '\n'
			<< "\n[Error Info]\n" << GetErrorInfo() << "\n\n"
			<< GetOriginString();;
		m_whatBuffer = oss.str();
		return m_whatBuffer.c_str();
	}
}