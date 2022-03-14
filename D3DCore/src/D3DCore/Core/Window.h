#pragma once
#include "D3DCWindows.h"
#include "D3DCException.h"
#include "D3DCore/Events/Event.h"
#include <optional>

namespace d3dcore
{
	class Window
	{
		using EventCallbackFn = std::function<void(Event&)>;

	private:
		class WindowClass
		{
		private:
			const std::string m_className;
			HINSTANCE m_hInst;

		public:
			WindowClass(const std::string& windowClassName);
			~WindowClass();
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator=(const WindowClass&) = delete;

			const std::string& GetName() const { return m_className; }
			HINSTANCE GetInstance() const { return m_hInst; }
		};

		struct WindowData
		{
			std::string title;
			uint32_t width = 0;
			uint32_t height = 0;
			bool shouldClose = false;
			bool isMinimized = false;
			bool isFocus = true;
			EventCallbackFn eventCallbackFn;
		};

	public:
		Window(uint32_t width, uint32_t height, const std::string& name);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		void SetTitle(const std::string& title);
		bool ShoudClose() const { return m_data.shouldClose; }
		inline void SetEventCallback(const EventCallbackFn& callback) { m_data.eventCallbackFn = callback; }
		inline void Close() { m_data.shouldClose = true; }

		uint32_t GetWidth() const { return m_data.width; }
		uint32_t GetHeight() const { return m_data.height; }
		bool IsFocus() const { return m_data.isFocus; }
		HWND GetNativeWindow() const { return m_hWnd; }
		static void PollEvents();

	private:
		LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		static LRESULT WINAPI HandleMessageSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		static LRESULT WINAPI HandleMessageCaller(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		WindowClass m_windowClass;
		WindowData m_data;
		HWND m_hWnd;





	// Exceptions stuff
	public:
		class Exception : public D3DCException
		{
			using D3DCException::D3DCException;
		public:
			static std::string TranslateErrorCode(HRESULT hr);
		};

		class HrException : public Exception
		{
		public:
			HrException(int line, const std::string& file, HRESULT hr);
			virtual const char* what() const override;

			HRESULT GetErrorCode() const { return m_hr; }
			std::string GetErrorDescription() const { return TranslateErrorCode(m_hr); }
			virtual const char* GetType() const override { return "Window Exception"; }

		private:
			HRESULT m_hr;
		};
	};
}

#define D3DC_WND_EXCEPT(hr) d3dcore::Window::HrException(__LINE__, __FILE__, (hr))
#define D3DC_WND_LAST_EXCEPT() D3DC_WND_EXCEPT(GetLastError())