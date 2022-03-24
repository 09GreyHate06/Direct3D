#pragma once
#include "D3DCore/Events/MouseEvent.h"
#include "Camera.h"
#include <DirectXMath.h>

namespace d3dcore::utils
{
	//struct EditorCameraDesc
	//{
	//	float fov = 45.0f, aspect = 1280.0f / 720.0f, nearPlane = 0.1f, farPlane = 1000.0f;
	//	float yaw = 0.0f, pitch = 0.0f, distance = 20.0f;
	//	float viewportWidth = 1280.0f, viewportHeight = 720.0f;
	//	DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	//	DirectX::XMFLOAT3 focalPoint = { 0.0f, 0.0f, 0.0f };
	//};

	class EditorCamController
	{
	public:
		EditorCamController();
		EditorCamController(Camera* context, float viewportWidth = 1280.0f, float viewportHeight = 720.0f, float distance = 10.0f, const DirectX::XMFLOAT3& focalPoint = { 0.0f ,0.0f, 0.0f });
		~EditorCamController() = default;

		void SetContext(Camera* context);

		void OnEvent(Event& event);
		void OnUpdate();

		inline void Reset() { m_reset = true; }
		void SetViewportSize(float width, float height);

	private:
		bool OnMouseScroll(MouseScrollEvent& event);

		void MousePan(const DirectX::XMFLOAT2& delta);
		void MouseRotate(const DirectX::XMFLOAT2& delta);
		void MouseZoom(float delta);
		void MouseFreeZoom(float delta);

		DirectX::XMFLOAT3 CalculatePosition() const;
		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;


		Camera* m_context = nullptr;

		bool m_reset = false;
		float m_viewportWidth = 0.0f;
		float m_viewportHeight = 0.0f;
		float m_distance = 0.0f;
		DirectX::XMFLOAT3 m_focalPoint = { 0.0f, 0.0f, 0.0f };

		DirectX::XMFLOAT2 m_initialMousePos = { 0.0f, 0.0f };
		DirectX::XMFLOAT4X4 m_projection;
		DirectX::XMFLOAT4X4 m_view;
	};
}