#pragma once
#include "D3DCore.h"
#include "D3DCore/Utils/Camera.h"
#include "D3DCore/Utils/EditorCamController.h"
#include "Scene/System/Rendering/RenderingSystem.h"
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
	d3dcore::Entity CreateCube();
	d3dcore::Entity CreatePlane();

	void ShowRenderer();
	void ShowRendererSettings();

	d3dcore::utils::Camera m_camera;
	d3dcore::utils::EditorCamController m_editorCamController;

	std::unique_ptr<d3dcore::Scene> m_scene;
	std::unique_ptr<RenderingSystem> m_renderingSystem;
	SceneHierarchyPanel m_sceneHierarchyPanel;

	bool m_viewportFocus = true;
	float m_sceneViewportWidth = 1280.0f;
	float m_sceneViewportHeight = 720.0f;

	bool m_showRenderer = false;
	bool m_showRendererSettings = false;
	bool m_depthEnable = true;
	bool m_faceCullEnable = true;
	bool m_blendEnable = true;
};