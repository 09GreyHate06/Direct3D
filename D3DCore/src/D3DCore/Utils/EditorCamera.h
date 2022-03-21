#pragma once
#include "D3DCore/Events/MouseEvent.h"
#include <DirectXMath.h>

namespace d3dcore::utils
{
	struct EditorCameraDesc
	{
		float fov = 45.0f, aspect = 1280.0f / 720.0f, nearPlane = 0.1f, farPlane = 1000.0f;
		float yaw = 0.0f, pitch = 0.0f, distance = 20.0f;
		float viewportWidth = 1280.0f, viewportHeight = 720.0f;
		DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 focalPoint = { 0.0f, 0.0f, 0.0f };
	};

	class EditorCamera
	{
	public:
		EditorCamera() = default;
		EditorCamera(const EditorCameraDesc& desc);
		~EditorCamera() = default;

		void OnEvent(Event& event);
		void OnUpdate();

		void SetCamera(const EditorCameraDesc& desc);
		const EditorCameraDesc& GetDesc() const { return m_desc; }

		DirectX::XMMATRIX GetViewMatrix() const;
		DirectX::XMMATRIX GetProjectionMatrix() const;
		DirectX::XMVECTOR GetUpDirection() const;
		DirectX::XMVECTOR GetRightDirection() const;
		DirectX::XMVECTOR GetForwardDirection() const;
		DirectX::XMVECTOR GetOrientation() const;

		inline void Reset() { m_reset = true; }
		inline void SetViewportSize(float width, float height) { m_desc.viewportWidth = width; m_desc.viewportHeight = height; UpdateProjectionMatrix(); }

	private:
		void UpdateProjectionMatrix();
		void UpdateViewMatrix();

		bool OnMouseScroll(MouseScrollEvent& event);

		void MousePan(const DirectX::XMFLOAT2& delta);
		void MouseRotate(const DirectX::XMFLOAT2& delta);
		void MouseZoom(float delta);

		DirectX::XMFLOAT3 CalculatePosition() const;
		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;


		EditorCameraDesc m_desc = {};

		bool m_reset = false;
		DirectX::XMFLOAT2 m_initialMousePos = { 0.0f, 0.0f };
		DirectX::XMFLOAT4X4 m_projection;
		DirectX::XMFLOAT4X4 m_view;
	};
}