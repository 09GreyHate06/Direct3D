#include "d3dcpch.h"
#include "Window.h"
#include "D3DCore/Events/ApplicationEvent.h"
#include "D3DCore/Events/KeyEvent.h"
#include "D3DCore/Events/MouseEvent.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace d3dcore
{
	Window::WindowClass::WindowClass(const std::string& windowClassName)
		: m_className(windowClassName), m_hInst(GetModuleHandleA(nullptr))
	{
		WNDCLASSEXA wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEXA);
		wcex.style = CS_OWNDC;
		wcex.lpfnWndProc = HandleMessageSetup;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = m_hInst;
		wcex.hIcon = nullptr;
		wcex.hCursor = nullptr;
		wcex.hbrBackground = nullptr;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = m_className.c_str();
		wcex.hIconSm = nullptr;

		RegisterClassExA(&wcex);
	}

	Window::WindowClass::~WindowClass()
	{
		UnregisterClassA(m_className.c_str(), m_hInst);
	}



	Window::Window(uint32_t width, uint32_t height, const std::string& name)
		: m_windowClass(name)
	{
		m_data.width = width;
		m_data.height = height;
		m_data.title = name;

		RECT rect;
		rect.left = 100;
		rect.right = m_data.width + rect.left;
		rect.top = 100;
		rect.bottom = m_data.height + rect.top;

		if (AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE) == FALSE)
			throw D3DC_WND_LAST_EXCEPT();

		m_hWnd = CreateWindowExA(
			0,
			m_windowClass.GetName().c_str(),
			m_data.title.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			rect.right - rect.left, rect.bottom - rect.top,
			nullptr,
			nullptr,
			m_windowClass.GetInstance(),
			this
		);

		if (!m_hWnd)
			throw D3DC_WND_LAST_EXCEPT();

		ShowWindow(m_hWnd, SW_SHOWDEFAULT);
	}

	Window::~Window()
	{
		DestroyWindow(m_hWnd);
	}

	void Window::SetTitle(const std::string& title)
	{
		m_data.title = title;
		SetWindowTextA(m_hWnd, title.c_str());
	}

	void Window::PollEvents()
	{
		MSG msg;
		while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
	}

	LRESULT Window::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		//if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		//	return true;

		switch (msg)
		{
		case WM_CLOSE:
		{
			m_data.shouldClose = true;

			if (!m_data.eventCallbackFn) return 0;
			m_data.eventCallbackFn(WindowCloseEvent());
			return 0;
		}
		case WM_KILLFOCUS: 
		{
			m_data.isFocus = false;
			if (!m_data.eventCallbackFn) break;
			m_data.eventCallbackFn(WindowLostFocusEvent());
			break;
		}
		case WM_SETFOCUS:
		{
			m_data.isFocus = true;
			if (!m_data.eventCallbackFn) break;
			m_data.eventCallbackFn(WindowFocusEvent());
			break;
		}
		case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED)
				m_data.isMinimized = true;
			else
				m_data.isMinimized = false;

			UINT width = LOWORD(lParam);
			UINT height = HIWORD(lParam);
			m_data.width = width;
			m_data.height = height;

			if (!m_data.eventCallbackFn) break;
			m_data.eventCallbackFn(WindowResizeEvent(width, height));
			break;
		}
		/****************************** KEYBOARD MESSAGES *************************************/
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			if (!m_data.eventCallbackFn) break;

			KeyCode key = static_cast<KeyCode>(wParam);
			bool repeat = (lParam & (1 << 30)); // todo figure out how to get repeat count
			m_data.eventCallbackFn(KeyPressedEvent(key, repeat));
			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			if (!m_data.eventCallbackFn) break;

			KeyCode key = static_cast<KeyCode>(wParam);
			m_data.eventCallbackFn(KeyReleasedEvent(key));
			break;
		}
		case WM_CHAR:
		{
			if (!m_data.eventCallbackFn) break;

			KeyCode c = static_cast<KeyCode>(wParam);
			m_data.eventCallbackFn(KeyTypedEvent(c));
			break;
		}
		/************************** END KEYBOARD MESSAGES *************************************/

		/********************************* MOUSE MESSAGES *************************************/

		case WM_MOUSEMOVE:
		{
			if (!m_data.eventCallbackFn) break;

			POINTS p = MAKEPOINTS(lParam);
			m_data.eventCallbackFn(MouseMovedEvent(p.x, p.y));
			break;
		}
		case WM_LBUTTONDOWN:
		{
			if (!m_data.eventCallbackFn) break;
			m_data.eventCallbackFn(MouseButtonPressedEvent(Mouse::LeftButton));
			break;
		}
		case WM_RBUTTONDOWN:
		{
			if (!m_data.eventCallbackFn) break;
			m_data.eventCallbackFn(MouseButtonPressedEvent(Mouse::RightButton));
			break;
		}
		case WM_MBUTTONDOWN:
		{
			if (!m_data.eventCallbackFn) break;
			m_data.eventCallbackFn(MouseButtonPressedEvent(Mouse::MiddleButton));
			break;
		}
		case WM_LBUTTONUP:
		{
			if (!m_data.eventCallbackFn) break;
			m_data.eventCallbackFn(MouseButtonReleasedEvent(Mouse::LeftButton));
			break;
		}
		case WM_RBUTTONUP:
		{
			if (!m_data.eventCallbackFn) break;
			m_data.eventCallbackFn(MouseButtonReleasedEvent(Mouse::RightButton));
			break;
		}
		case WM_MBUTTONUP:
		{
			if (!m_data.eventCallbackFn) break;
			m_data.eventCallbackFn(MouseButtonReleasedEvent(Mouse::MiddleButton));
			break;
		}
		case WM_MOUSEWHEEL:
		{
			if (!m_data.eventCallbackFn) break;

			int delta = GET_WHEEL_DELTA_WPARAM(wParam);
			float axisY = 0.0f;
			if (delta >= WHEEL_DELTA)
				axisY = 1.0f;
			else if (delta <= -WHEEL_DELTA)
				axisY = -1.0f;

			m_data.eventCallbackFn(MouseScrollEvent(0.0f, axisY));
			break;
		}
		case WM_MOUSEHWHEEL:
		{
			if (!m_data.eventCallbackFn) break;

			int delta = GET_WHEEL_DELTA_WPARAM(wParam);
			float axisX = 0.0f;
			if (delta >= WHEEL_DELTA)
				axisX = 1.0f;
			else if (delta <= -WHEEL_DELTA)
				axisX = -1.0f;

			m_data.eventCallbackFn(MouseScrollEvent(axisX, 0.0f));
			break;
		}
		}

		return DefWindowProcA(hWnd, msg, wParam, lParam);
	}

	LRESULT WINAPI Window::HandleMessageSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
		if (msg == WM_NCCREATE)
		{
			// extract ptr to window class from creation data
			const CREATESTRUCTA* const createStruct = reinterpret_cast<CREATESTRUCTA*>(lParam);
			Window* const window = static_cast<Window*>(createStruct->lpCreateParams);
			// set WinAPI-managed user data to store ptr to window instance
			SetWindowLongPtrA(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
			// set message proc to normal (non-setup) handler now that setup is finished
			SetWindowLongPtrA(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Window::HandleMessageCaller));
			// forward message to window instance handler
			return window->HandleMessage(hWnd, msg, wParam, lParam);
		}

		// if we get a message before the WM_NCCREATE message, handle with default handler
		return DefWindowProcA(hWnd, msg, wParam, lParam);
	}

	LRESULT WINAPI Window::HandleMessageCaller(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// retrieve ptr to window instance
		Window* const window = reinterpret_cast<Window*>(GetWindowLongPtrA(hWnd, GWLP_USERDATA));
		// forward message to window instance handler
		return window->HandleMessage(hWnd, msg, wParam, lParam);
	}






	// Exception stuffs

	std::string Window::Exception::TranslateErrorCode(HRESULT hr)
	{
		char* msgBuffer = nullptr;
		DWORD msgLen = FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPSTR>(&msgBuffer), 0, nullptr
		);

		if (msgLen == 0)
			return "Unidentified Error";

		std::string errorString = msgBuffer;
		LocalFree(msgBuffer);

		return errorString;
	}


	Window::HrException::HrException(int line, const std::string& file, HRESULT hr)
		: Exception(line, file), m_hr(hr)
	{
	}

	const char* Window::HrException::what() const
	{
		std::ostringstream oss;
		oss << GetType() << '\n'
			<< "[ERROR CODE] " << GetErrorCode() << '\n'
			<< "[DESCRIPTION] " << GetErrorDescription() << '\n'
			<< GetOriginString();

		m_whatBuffer = oss.str();
		return m_whatBuffer.c_str();
	}


}