#pragma once
#include "D3DCore.h"
#include "Scene/System/RenderingSystem.h"
#include "SceneHierarchyPanel.h"

class EditorLayer : public d3dcore::Layer
{
public:
	virtual void OnAttach() override;
	virtual void OnEvent(d3dcore::Event& event) override;
	virtual void OnUpdate() override;
	virtual void OnDetach() override;
	virtual void OnImGuiRender() override;

private:
	void CreateCube();
	void CreatePlane();

	d3dcore::utils::EditorCamera m_camera;
	std::unique_ptr<d3dcore::Scene> m_scene;
	std::unique_ptr<RenderingSystem> m_renderingSystem;
	SceneHierarchyPanel m_sceneHierarchyPanel;

	std::shared_ptr<d3dcore::VertexBuffer> m_cubeVB;
	std::shared_ptr<d3dcore::VertexBuffer> m_planeVB;
	std::shared_ptr<d3dcore::IndexBuffer> m_cubeIB;
	std::shared_ptr<d3dcore::IndexBuffer> m_planeIB;

	bool m_viewportFocus = true;
	float m_sceneViewportWidth = 1280.0f;
	float m_sceneViewportHeight = 720.0f;
};