#pragma once
#include "D3DCore.h"

class SceneHierarchyPanel
{
public:
	SceneHierarchyPanel() = default;
	SceneHierarchyPanel(d3dcore::Scene* context);

	void SetContext(d3dcore::Scene* context);
	void OnImGuiRender();
	void SetSelectedEntity(d3dcore::Entity entity);

private:
	void DrawEntityNode(d3dcore::Entity entity, bool child);
	void DrawComponents(d3dcore::Entity entity);

	d3dcore::Scene* m_context = nullptr;
	d3dcore::Entity m_selectedEntity;

	std::vector<d3dcore::Entity> m_entitiesToDestroy;
};