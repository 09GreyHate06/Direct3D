#include "d3dcpch.h"
#include "DxgiInfoManager.h"
#include "D3DCore/Core/Window.h"
#include "D3DCore/Renderer/D3DContext.h"

#pragma comment(lib, "dxguid.lib")

//#define D3DC_GFX_THROW_NOINFO(hrcall) if(FAILED(hr = (hrcall))) throw d3dcore::Graphics::HrException(__LINE__, __FILE__, hr)

namespace d3dcore
{
	DxgiInfoManager::DxgiInfoManager()
	{
		// define function signature of DXGIGetDebugInterface
		typedef HRESULT(WINAPI* DXGIGetDebugInterface)(REFIID, void**);

		// load dll that contains the function DXGIGetDebugInterface
		const auto hModDxgiDebug = LoadLibraryExA("dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
		if (!hModDxgiDebug)
		{
			throw D3DC_WND_LAST_EXCEPT();
		}

		// get address of DXGIGetDebugInterface in dll
		const auto DxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>(
			reinterpret_cast<void*>(GetProcAddress(hModDxgiDebug, "DXGIGetDebugInterface"))
			);

		if (!DxgiGetDebugInterface)
		{
			throw D3DC_WND_LAST_EXCEPT();
		}

		HRESULT hr;
		D3DC_GFX_THROW_NOINFO(DxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), &m_dxgiInfoQueue));
	}

	void DxgiInfoManager::Set()
	{
		// set the index (next) so that the next all to GetMessages()
		// will only get errors generated after this call
		m_next = m_dxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
	}

	std::vector<std::string> DxgiInfoManager::GetMessages() const
	{
		std::vector<std::string> messages;
		const auto end = m_dxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
		for (uint64_t i = m_next; i < end; i++)
		{
			HRESULT hr;
			SIZE_T messageLength = 0;
			// get the size of message i in bytes
			D3DC_GFX_THROW_NOINFO(m_dxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageLength));
			// allocate memory for message
			auto bytes = std::make_unique<byte[]>(messageLength);
			auto message = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.get());
			// get the message and push its description into the vector
			D3DC_GFX_THROW_NOINFO(m_dxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, message, &messageLength));
			messages.emplace_back(message->pDescription);
		}

		return messages;
	}
}