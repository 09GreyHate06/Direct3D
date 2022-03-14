#include "d3dcpch.h"
#include "Input.h"

#include "Application.h"
#include <imgui.h>

namespace d3dcore
{
	bool Input::GetKey(const KeyCode key)
	{
		Application& app = Application::Get();

		//if (app.GetImGuiLayer()->IsBlockingEvents())
		//{
		//	ImGuiIO& io = ImGui::GetIO();
		//	if (io.WantCaptureKeyboard) 
		//		return false;
		//}

		if (!app.GetWindow().IsFocus()) return false;

		return (GetAsyncKeyState(key) & 0x8000) != 0;
	}

	bool Input::GetMouseButton(const MouseCode button)
	{
		Application& app = Application::Get();

		//if (app.GetImGuiLayer()->IsBlockingEvents())
		//{
		//	ImGuiIO& io = ImGui::GetIO();
		//	if (io.WantCaptureMouse) return false;
		//}

		if (!app.GetWindow().IsFocus()) return false;

		return (GetAsyncKeyState(button) & 0x8000) != 0;
	}

	DirectX::XMFLOAT2 Input::GetMousePos()
	{
		HWND windowHandle = Application::Get().GetWindow().GetNativeWindow();
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(windowHandle, &p);
		return { static_cast<float>(p.x), static_cast<float>(p.y) };
	}

	float Input::GetMouseX()
	{
		auto [x, y] = GetMousePos();
		return x;
	}

	float Input::GetMouseY()
	{
		auto [x, y] = GetMousePos();
		return y;
	}

}
