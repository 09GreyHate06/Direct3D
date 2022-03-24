#pragma once
#include "Camera.h"
#include "D3DCore/Events/Event.h"

namespace d3dcore::utils
{
	class FPSCamController
	{
	public:
		FPSCamController(Camera* context, float moveSpeed = 5.0f, float mouseSensitivity = 2.0f);
		FPSCamController();

		void SetContext(Camera* context);
		void SetMoveSpeed(float speed);
		void SetRotateSpeed(float speed);

		void OnUpdate();
		void OnEvent(Event& event);

	private:
		void ProcessMovement();
		void ProcessRotation();

		Camera* m_context;
		float m_mouseSensitivity;
		float m_moveSpeed;

		bool m_enabled = false;

		DirectX::XMFLOAT2 m_mouseOrigin;
	};
}