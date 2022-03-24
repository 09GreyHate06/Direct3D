#include "d3dcpch.h"
#include "FPSCamController.h"
#include "D3DCore/Core/Input.h"
#include "D3DCore/Core/Time.h"
#include "D3DCore/Events/KeyEvent.h"

using namespace DirectX;

namespace d3dcore::utils
{
	FPSCamController::FPSCamController(Camera* context, float moveSpeed, float mouseSensitivity)
		: m_context(context), m_moveSpeed(moveSpeed), m_mouseSensitivity(mouseSensitivity)
	{
	}

	FPSCamController::FPSCamController()
		: FPSCamController(nullptr)
	{
	}

	void FPSCamController::SetContext(Camera* context)
	{
		m_context = context;
	}

	void FPSCamController::SetMoveSpeed(float speed)
	{
		m_moveSpeed = speed;
	}

	void FPSCamController::SetRotateSpeed(float speed)
	{
		m_mouseSensitivity = speed;
	}

	void FPSCamController::OnUpdate()
	{
		if (!m_context || !m_enabled) return;

		ProcessMovement();
		ProcessRotation();
	}

	void FPSCamController::OnEvent(Event& event)
	{
		if (event.GetEventType() == EventType::KeyPressed)
		{
			auto& e = static_cast<KeyPressedEvent&>(event);
			if (e.GetKeyCode() == Key::F)
			{
				m_enabled = !m_enabled;
				ShowCursor(m_enabled);
			}
		}
	}

	void FPSCamController::ProcessMovement()
	{
		float velocity = m_moveSpeed * Time::Delta();
		XMVECTOR curPos = XMLoadFloat3(&m_context->GetPosition());
		
		if (Input::GetKey(Key::W) || Input::GetKey(Key::Up))
			curPos += m_context->GetForwardDirection() * velocity;
		if (Input::GetKey(Key::S) || Input::GetKey(Key::Down))
			curPos -= m_context->GetForwardDirection() * velocity;
		if (Input::GetKey(Key::D) || Input::GetKey(Key::Right))
			curPos += m_context->GetRightDirection() * velocity;
		if (Input::GetKey(Key::A) || Input::GetKey(Key::Left))
			curPos -= m_context->GetRightDirection() * velocity;

		XMFLOAT3 pos;
		XMStoreFloat3(&pos, curPos);
		m_context->SetPosition(pos);
	}

	void FPSCamController::ProcessRotation()
	{
		XMFLOAT2 curMousePos = Input::GetMousePos();
		XMFLOAT2 offset = {};
		offset.x = curMousePos.x - m_mouseOrigin.x;
		offset.y = curMousePos.y - m_mouseOrigin.y;

		offset.x *= m_mouseSensitivity * Time::Delta();
		offset.y *= m_mouseSensitivity * Time::Delta();

		float yaw = m_context->GetYaw();
		float pitch = m_context->GetPitch();

		yaw += offset.y;
		pitch = std::clamp(pitch + offset.x, -89.0f, 89.0f);
		m_mouseOrigin = curMousePos;

		m_context->SetYaw(yaw);
		m_context->SetPitch(pitch);
	}
}