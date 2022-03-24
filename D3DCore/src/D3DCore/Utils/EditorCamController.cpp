#include "d3dcpch.h"
#include "EditorCamController.h"
#include "D3DCore/Core/Input.h"

using namespace DirectX;

namespace d3dcore::utils
{
	EditorCamController::EditorCamController(Camera* context, float viewportWidth, float viewportHeight, float distance, const DirectX::XMFLOAT3& focalPoint)
		: m_context(context), m_viewportWidth(viewportWidth), m_viewportHeight(viewportHeight), m_distance(distance), m_focalPoint(focalPoint)
	{
	}

	EditorCamController::EditorCamController()
		: EditorCamController(nullptr)
	{
	}

	void EditorCamController::SetContext(Camera* context)
	{
		m_context = context;
		m_context->SetPosition(CalculatePosition());
	}

	void EditorCamController::OnEvent(Event& event)
	{
		if (!m_context) return;

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseScrollEvent>(D3DC_BIND_EVENT_FN(EditorCamController::OnMouseScroll));
	}

	void EditorCamController::OnUpdate()
	{
		if (!m_context) return;

		if (m_reset)
		{
			m_initialMousePos = Input::GetMousePos();
			m_reset = false;
		}

		if (Input::GetKey(Key::LeftMenu))
		{
			XMVECTOR initialMousePos = XMLoadFloat2(&m_initialMousePos);
			XMVECTOR mouse = XMLoadFloat2(&Input::GetMousePos());
			XMVECTOR delta = (mouse - initialMousePos) * 0.003f;
			XMStoreFloat2(&m_initialMousePos, mouse);

			XMFLOAT2 fdelta;
			XMStoreFloat2(&fdelta, delta);

			if (Input::GetKey(Key::LeftControl) && Input::GetMouseButton(Mouse::LeftButton))
				MousePan(fdelta);
			else if (Input::GetMouseButton(Mouse::LeftButton))
				MouseRotate(fdelta);

			m_context->SetPosition(CalculatePosition());
		}
	}

	void EditorCamController::SetViewportSize(float width, float height)
	{
		if (!m_context) return;

		m_viewportWidth = width;
		m_viewportHeight = height;
		m_context->SetAspect(width / height);
	}

	bool EditorCamController::OnMouseScroll(MouseScrollEvent& event)
	{
		float delta = event.GetAxisY() * 0.1f;

		if (Input::GetKey(Key::LeftMenu))
			MouseFreeZoom(delta);
		else
			MouseZoom(delta);

		return false;
	}

	void EditorCamController::MousePan(const DirectX::XMFLOAT2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		XMVECTOR focalPoint = XMLoadFloat3(&m_focalPoint);
		focalPoint += -m_context->GetRightDirection() * delta.x * xSpeed * m_distance;
		focalPoint += m_context->GetUpDirection() * delta.y * ySpeed * m_distance;
		XMStoreFloat3(&m_focalPoint, focalPoint);
	}

	void EditorCamController::MouseRotate(const DirectX::XMFLOAT2& delta)
	{
		auto yaw = m_context->GetYaw();
		auto pitch = m_context->GetPitch();

		float yawSign = XMVectorGetY(m_context->GetUpDirection()) < 0.0f ? 1.0f : -1.0f;
		yaw += yawSign * delta.x * RotationSpeed();
		pitch += delta.y * RotationSpeed();

		m_context->SetYaw(yaw);
		m_context->SetPitch(pitch);
	}

	void EditorCamController::MouseZoom(float delta)
	{
		m_distance -= delta * ZoomSpeed();
		if (m_distance < 1.0f)
		{
			XMVECTOR focalPoint = XMLoadFloat3(&m_focalPoint);
			focalPoint += m_context->GetForwardDirection();
			XMStoreFloat3(&m_focalPoint, focalPoint);
			m_distance = 1.0f;
		}

		m_context->SetPosition(CalculatePosition());
	}

	void EditorCamController::MouseFreeZoom(float delta)
	{
		XMVECTOR focalPoint = XMLoadFloat3(&m_focalPoint);
		focalPoint += m_context->GetForwardDirection() * delta * ZoomSpeed();
		XMStoreFloat3(&m_focalPoint, focalPoint);
		m_context->SetPosition(CalculatePosition());
	}

	DirectX::XMFLOAT3 EditorCamController::CalculatePosition() const
	{
		XMVECTOR focalPoint = XMLoadFloat3(&m_focalPoint);
		XMVECTOR pos = focalPoint - m_context->GetForwardDirection() * m_distance;

		XMFLOAT3 fpos;
		XMStoreFloat3(&fpos, pos);
		return fpos;
	}

	std::pair<float, float> EditorCamController::PanSpeed() const
	{
		float x = std::min(m_viewportWidth / 1000.0f, 2.4f); // max 2.4f;
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_viewportHeight / 1000.0f, 2.4f); // max 2.4
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamController::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamController::ZoomSpeed() const
	{
		float distance = m_distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}
}