#pragma once
#include "D3DCore/Core/D3DCWindows.h"
#include "D3DCore/Core/D3DCException.h"
#include "DXError/DxgiInfoManager.h"

#include <d3d11.h>

namespace d3dcore
{
	class D3DContext
	{
	public:
		static ID3D11Device* const GetDevice() { return s_device; }
		static ID3D11DeviceContext* const GetDeviceContext() { return s_deviceContext; }
		static IDXGISwapChain* const GetSwapChain() { return s_swapChain; }

		static uint32_t GetSampleCount() { return s_sampleCount; }
		static uint32_t GetSampleQuality() { return s_sampleQuality; }

#ifdef D3DC_DEBUG
		static DxgiInfoManager& GetInfoManager() { return s_infoManager; }
#endif // D3DC_DEBUG


	private:
		static void Init(HWND hWnd, uint32_t sampleCount, uint32_t sampleQuality);
		static void Shutdown();

#ifdef D3DC_DEBUG
		static DxgiInfoManager s_infoManager;
#endif // D3DC_DEBUG

		static ID3D11Device* s_device;
		static ID3D11DeviceContext* s_deviceContext;
		static IDXGISwapChain* s_swapChain;
		
		static uint32_t s_sampleCount;
		static uint32_t s_sampleQuality;

		friend class Renderer;


		// exception stuffs
	public:
		class Exception : public D3DCException
		{
			using D3DCException::D3DCException;
		};

		class HrException : public Exception
		{
		public:
			HrException(int line, const std::string& file, HRESULT hr, const std::vector<std::string>& infoMessages = {});
			virtual const char* what() const override;
			virtual const char* GetType() const override { return "D3DCContext Exception"; }
			HRESULT GetErrorCode() const { return m_hr; }
			std::string GetErrorString() const;
			std::string GetErrorDescription() const;
			const std::string& GetErrorInfo() const { return m_info; }

		private:
			HRESULT m_hr;
			std::string m_info;
		};

		class InfoException : public Exception
		{
		public:
			InfoException(int line, const std::string& file, const std::vector<std::string>& infoMessages);
			virtual const char* what() const override;
			virtual const char* GetType() const { return "D3DContext Info Exception"; }
			const std::string& GetErrorInfo() const { return m_info; }

		private:
			std::string m_info;
		};

		class DeviceRemovedException : public HrException
		{
			using HrException::HrException;
		public:
			virtual const char* GetType() const override { return "D3DContext Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)"; }

		private:
			std::string m_reason;
		};
	};
}

// graphics exception checking/throwing macros (some with dxgi infos)
#define D3DC_CONTEXT_EXCEPT_NOINFO(hr) d3dcore::D3DContext::HrException(__LINE__, __FILE__, (hr))
#define D3DC_CONTEXT_THROW_NOINFO(hrcall) if(FAILED(hr = (hrcall))) throw d3dcore::D3DContext::HrException(__LINE__, __FILE__, hr)

#ifdef D3DC_DEBUG
#define D3DC_CONTEXT_EXCEPT(hr) d3dcore::D3DContext::HrException(__LINE__, __FILE__, (hr), d3dcore::D3DContext::GetInfoManager().GetMessages() )
#define D3DC_CONTEXT_THROW_INFO(hrcall) d3dcore::D3DContext::GetInfoManager().Set(); if( FAILED( hr = (hrcall) ) ) throw D3DC_CONTEXT_EXCEPT(hr)
#define D3DC_CONTEXT_DEVICE_REMOVED_EXCEPT(hr) d3dcore::D3DContext::DeviceRemovedException( __LINE__, __FILE__, (hr), d3dcore::D3DContext::GetInfoManager().GetMessages() )
#define D3DC_CONTEXT_THROW_INFO_ONLY(call) d3dcore::D3DContext::GetInfoManager().Set(); (call); { auto v = d3dcore::D3DContext::GetInfoManager().GetMessages(); if(!v.empty()) { throw d3dcore::D3DContext::InfoException(__LINE__, __FILE__, v); } } 
#else		 
#define D3DC_CONTEXT_EXCEPT(hr) d3dcore::D3DContext::HrException( __LINE__,__FILE__,(hr) )
#define D3DC_CONTEXT_THROW_INFO(hrcall) D3DC_CONTEXT_THROW_NOINFO(hrcall)
#define D3DC_CONTEXT_DEVICE_REMOVED_EXCEPT(hr) d3dcore::D3DContext::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#define D3DC_CONTEXT_THROW_INFO_ONLY(call) (call)
#endif // D3DC_DEBUG