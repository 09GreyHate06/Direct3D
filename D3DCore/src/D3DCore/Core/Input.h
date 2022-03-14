#pragma once
#include "KeyCodes.h"
#include "MouseCodes.h"

#include <DirectXMath.h>

namespace d3dcore
{
	class Input
	{
	public:
		static bool GetKey(const KeyCode key);
		static bool GetMouseButton(const MouseCode button);
		static DirectX::XMFLOAT2 GetMousePos();
		static float GetMouseX();
		static float GetMouseY();


	private:
		Input() = default;
	};
}