#include "d3dcpch.h"
#include "EditorCamera.h"
#include "D3DCore/Core/Input.h"

using namespace DirectX;

namespace d3dcore::utils
{
	EditorCamera::EditorCamera(const EditorCameraDesc& desc)
		: m_desc(desc)
	{
		UpdateProjectionMatrix();
		UpdateViewMatrix();
	}

	void EditorCamera::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseScrollEvent>(D3DC_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
	}

	void EditorCamera::OnUpdate()
	{
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
		}

		UpdateViewMatrix();
	}

	void EditorCamera::SetCamera(const EditorCameraDesc& desc)
	{
		m_desc = desc;
		UpdateProjectionMatrix();
		UpdateViewMatrix();
	}

	DirectX::XMMATRIX EditorCamera::GetViewMatrix() const
	{
		return XMLoadFloat4x4(&m_view);
	}

	DirectX::XMMATRIX EditorCamera::GetProjectionMatrix() const
	{
		return XMLoadFloat4x4(&m_projection);
	}

	DirectX::XMVECTOR EditorCamera::GetUpDirection() const
	{
		XMVECTOR v = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		return XMVector3Rotate(v, GetOrientation());
	}

	DirectX::XMVECTOR EditorCamera::GetRightDirection() const
	{
		XMVECTOR v = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		return XMVector3Rotate(v, GetOrientation());
	}

	DirectX::XMVECTOR EditorCamera::GetForwardDirection() const
	{
		XMVECTOR v = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		return XMVector3Rotate(v, GetOrientation());
	}

	DirectX::XMVECTOR EditorCamera::GetOrientation() const
	{
		return XMQuaternionRotationRollPitchYaw(m_desc.pitch, -m_desc.yaw, 0.0f);
	}

	void EditorCamera::UpdateProjectionMatrix()
	{
		m_desc.aspect = m_desc.viewportWidth / m_desc.viewportHeight;
		XMMATRIX projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_desc.fov), m_desc.aspect, m_desc.nearPlane, m_desc.farPlane);
		XMStoreFloat4x4(&m_projection, projection);
	}

	void EditorCamera::UpdateViewMatrix()
	{
		m_desc.position = CalculatePosition();
		XMMATRIX view = XMMatrixIdentity();
		view = XMMatrixRotationQuaternion(GetOrientation()) * XMMatrixTranslation(m_desc.position.x, m_desc.position.y, m_desc.position.z);
		view = XMMatrixInverse(nullptr, view);
		XMStoreFloat4x4(&m_view, view);
	}

	bool EditorCamera::OnMouseScroll(MouseScrollEvent& event)
	{
		float delta = event.GetAxisY() * 0.1f;
		MouseZoom(delta);
		UpdateViewMatrix();

		return false;
	}

	void EditorCamera::MousePan(const DirectX::XMFLOAT2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		XMVECTOR focalPoint = XMLoadFloat3(&m_desc.focalPoint);
		focalPoint += -GetRightDirection() * delta.x * xSpeed * m_desc.distance;
		focalPoint += GetUpDirection() * delta.y * ySpeed * m_desc.distance;
		XMStoreFloat3(&m_desc.focalPoint, focalPoint);
	}

	void EditorCamera::MouseRotate(const DirectX::XMFLOAT2& delta)
	{
		float yawSign = XMVectorGetY(GetUpDirection()) < 0.0f ? 1.0f : -1.0f;
		m_desc.yaw += yawSign * delta.x * RotationSpeed();
		m_desc.pitch += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_desc.distance -= delta * ZoomSpeed();
		if (m_desc.distance < 1.0f)
		{
			XMVECTOR focalPoint = XMLoadFloat3(&m_desc.focalPoint);
			focalPoint += GetForwardDirection();
			XMStoreFloat3(&m_desc.focalPoint, focalPoint);
			m_desc.distance = 1.0f;
		}
	}

	DirectX::XMFLOAT3 EditorCamera::CalculatePosition() const
	{
		XMVECTOR focalPoint = XMLoadFloat3(&m_desc.focalPoint);
		XMVECTOR pos = focalPoint - GetForwardDirection() * m_desc.distance;

		XMFLOAT3 fpos;
		XMStoreFloat3(&fpos, pos);
		return fpos;
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(m_desc.viewportWidth / 1000.0f, 2.4f); // max 2.4f;
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_desc.viewportHeight / 1000.0f, 2.4f); // max 2.4
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_desc.distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}
}