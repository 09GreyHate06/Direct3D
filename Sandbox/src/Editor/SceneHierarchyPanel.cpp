#include "SceneHierarchyPanel.h"
#include "Scene/Components/Components.h"
#include "Utils/FileDialog.h"

using namespace d3dcore;
using namespace DirectX;

static constexpr const char* s_entityDragDropID = "SceneHierarchyEntity";
static constexpr const char* s_entityDragOutsideID = "SceneHierarchyEntityOutside";
static float s_dragSpeed = 0.1f;

template<typename T, typename Func>
static void DrawComponent(const std::string& name, Entity entity, Func func)
{
	const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth 
		| ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
	ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

	if (entity.HasComponent<T>())
	{
		auto& component = entity.GetComponent<T>();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImGui::Separator();
		bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(T).hash_code()), treeNodeFlags, name.c_str());
		ImGui::PopStyleVar();
		ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
		if (ImGui::Button("+", ImVec2(lineHeight, lineHeight)))
		{
			ImGui::OpenPopup("ComponentSettings");
		}

		bool removeComponent = false;
		if (ImGui::BeginPopup("ComponentSettings"))
		{
			if (ImGui::MenuItem("Remove component"))
				removeComponent = true;

			ImGui::EndPopup();
		}

		if (open)
		{
			func(component);
			ImGui::TreePop();
		}

		if (removeComponent)
			entity.RemoveComponent<T>();
	}
}

SceneHierarchyPanel::SceneHierarchyPanel(Scene* scene)
{
	SetContext(scene);
}

void SceneHierarchyPanel::SetContext(d3dcore::Scene* scene)
{
	m_context = scene;
	m_selectedEntity = {};
}

void SceneHierarchyPanel::OnImGuiRender()
{
	while (!m_entitiesToDestroy.empty())
	{
		if (m_selectedEntity == m_entitiesToDestroy.back())
			m_selectedEntity = {};

		m_context->DestroyEntity(m_entitiesToDestroy.back());
		m_entitiesToDestroy.pop_back();
	}

	if (ImGui::Begin("Scene Hierarchy"))
	{
		m_context->GetRegistry().each([&](entt::entity entityID)
		{
			Entity entity = { entityID, m_context };
			ImGui::BeginChild("abc");
			DrawEntityNode(entity, false);
			ImGui::EndChild();

			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(s_entityDragDropID);
				if (payload)
				{
					Entity* draggedEntity = reinterpret_cast<Entity*>(payload->Data);
					m_context->SetEntityRelationship(*draggedEntity, Entity());
				}
				ImGui::EndDragDropTarget();
			}
		});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_selectedEntity = {};

		ImGui::BeginChild("abc");
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
				m_context->CreateEntity("Empty Entity");

			ImGui::EndPopup();
		}
		ImGui::EndChild();

		ImGui::End();
	}


	ImGui::Begin("Inspector");
	if (m_selectedEntity)
	{
		DrawComponents(m_selectedEntity);
	}
	ImGui::End();
}

void SceneHierarchyPanel::SetSelectedEntity(d3dcore::Entity entity)
{
	m_selectedEntity = entity;
}

void SceneHierarchyPanel::DrawEntityNode(d3dcore::Entity entity, bool child)
{
	auto& relationship = entity.GetComponent<RelationshipComponent>();
	if (relationship.parent && !child) return;

	auto& tag = entity.GetComponent<TagComponent>().tag;

	ImGuiTreeNodeFlags flags = ((m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0)
		| (relationship.first ? 0 : ImGuiTreeNodeFlags_Leaf) | ImGuiTreeNodeFlags_OpenOnArrow;

	flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
	bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<uint64_t>(static_cast<uint32_t>(entity))), flags, tag.c_str());
	if (ImGui::IsItemClicked())
	{
		m_selectedEntity = entity;
	}

	if (ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload(s_entityDragDropID, &entity, sizeof(Entity));
		ImGui::Text(tag.c_str());
		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(s_entityDragDropID);
		if (payload)
		{
			Entity* draggedEntity = reinterpret_cast<Entity*>(payload->Data);
			m_context->SetEntityRelationship(*draggedEntity, entity);
		}
		ImGui::EndDragDropTarget();
	}

	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Create Empty Entity"))
			m_context->CreateEntity("Empty Entity", entity);
		if (ImGui::MenuItem("Destroy Entity"))
			m_entitiesToDestroy.push_back(entity);
		ImGui::EndPopup();
	}

	if (opened)
	{
		Entity current = relationship.first;
		Entity next;
		while (current)
		{
			DrawEntityNode(current, true);
			if (!current) break;
			next = current.GetComponent<RelationshipComponent>().next;
			current = next;
		}
		ImGui::TreePop();
	}
}

void SceneHierarchyPanel::DrawComponents(d3dcore::Entity entity)
{
	if (entity.HasComponent<TagComponent>())
	{
		auto& tagComponent = entity.GetComponent<TagComponent>();
		char buffer[512];
		memset(buffer, 0, sizeof(buffer));
		memcpy(buffer, tagComponent.tag.c_str(), tagComponent.tag.size());
		if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
		{
			tagComponent.tag = buffer;
		}
	}

	ImGui::SameLine();
	ImGui::PushItemWidth(-1);

	if (ImGui::Button("Add Component"))
		ImGui::OpenPopup("AddComponent");

	if (ImGui::BeginPopup("AddComponent"))
	{
		if (ImGui::MenuItem("Mesh"))
		{
			if (!entity.HasComponent<MeshComponent>())
				m_selectedEntity.AddComponent<MeshComponent>();
			else
				D3DC_LOG_WARN(entity.GetComponent<TagComponent>().tag + " already have Mesh Component");

			ImGui::CloseCurrentPopup();
		}

		if (ImGui::MenuItem("Mesh Renderer"))
		{
			if (!entity.HasComponent<MeshRendererComponent>())
				m_selectedEntity.AddComponent<MeshRendererComponent>();
			else
				D3DC_LOG_WARN(entity.GetComponent<TagComponent>().tag + " already have Mesh Renderer Component");

			ImGui::CloseCurrentPopup();
		}

		if (ImGui::MenuItem("Material Component"))
		{
			if (!entity.HasComponent<MaterialComponent>())
				m_selectedEntity.AddComponent<MaterialComponent>();
			else
				D3DC_LOG_WARN(entity.GetComponent<TagComponent>().tag + " already have Material Component");

			ImGui::CloseCurrentPopup();
		}

		if (ImGui::MenuItem("Directional Light"))
		{
			if (!entity.HasComponent<DirectionalLightComponent>())
				m_selectedEntity.AddComponent<DirectionalLightComponent>();
			else
				D3DC_LOG_WARN(entity.GetComponent<TagComponent>().tag +" already have Directional Light Component");

			ImGui::CloseCurrentPopup();
		}

		if (ImGui::MenuItem("Point Light"))
		{
			if (!entity.HasComponent<PointLightComponent>())
				m_selectedEntity.AddComponent<PointLightComponent>();
			else
				D3DC_LOG_WARN(entity.GetComponent<TagComponent>().tag + " already have Point Light Component");

			ImGui::CloseCurrentPopup();
		}

		if (ImGui::MenuItem("Spot Light"))
		{
			if (!entity.HasComponent<SpotLightComponent>())
				m_selectedEntity.AddComponent<SpotLightComponent>();
			else
				D3DC_LOG_WARN(entity.GetComponent<TagComponent>().tag + " already have Spot Light Component");

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::PopItemWidth();

	//DrawComponent<RelationshipComponent>("Relationship", entity, [](auto& component)
	//{
	//	ImGui::LabelText("Parent", component.parent ? component.parent.GetComponent<TagComponent>().tag.c_str() : "N/A");
	//	ImGui::LabelText("First child", component.first ? component.first.GetComponent<TagComponent>().tag.c_str() : "N/A");
	//	ImGui::LabelText("Prev sibling", component.prev ? component.prev.GetComponent<TagComponent>().tag.c_str() : "N/A");
	//	ImGui::LabelText("Next sibling", component.next ? component.next.GetComponent<TagComponent>().tag.c_str() : "N/A");
	//});

	DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
	{
		ImGui::DragFloat3("Position", &component.position.x, s_dragSpeed);
		ImGui::DragFloat3("Rotation", &component.rotation.x, s_dragSpeed);
		ImGui::DragFloat3("Scale", &component.scale.x, s_dragSpeed);
	});

	DrawComponent<MeshComponent>("Mesh", entity, [](auto& component)
	{

	});

	DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [](auto& component)
	{
		const char* typeStr[] = { "Line list", "Line strip", "Point list", "Triangle list", "Triangle strip" };
		int index = 0;

		switch (component.topology)
		{
		case Topology::LineList:	   index = 0; break;
		case Topology::LineStrip:	   index = 1; break;
		case Topology::PointList:	   index = 2; break;
		case Topology::TriangleList:   index = 3; break;
		case Topology::TriangleStrip:  index = 4; break;
		}

		if (ImGui::Combo("Primitive", &index, typeStr, IM_ARRAYSIZE(typeStr)))
		{
			switch (index)
			{
			case 0: component.topology = Topology::LineList;		 break;
			case 1: component.topology = Topology::LineStrip;	 break;
			case 2: component.topology = Topology::PointList;	 break;
			case 3: component.topology = Topology::TriangleList;  break;
			case 4: component.topology = Topology::TriangleStrip; break;
			}
		}

		ImGui::Checkbox("Receive Light", &component.receiveLight);
	});

	DrawComponent<MaterialComponent>("Material", entity, [](auto& component)
	{
		int id = 0;
		ImGui::Text("Diffuse Map");
		ImGui::PushID(id++);
		if (ImGui::Button("Browse"))
		{
			std::string newTexFilepath = FileDialog::OpenFileDialog("Image Files (*.png, *.jpg, *.jpeg, *.tga)\0*.png;*.jpg;\0*.jpeg;\0*.tga\0");
			if (!newTexFilepath.empty())
			{
				auto tex = Texture2D::Create(newTexFilepath, false, Texture2DDesc());
				component.diffuseMap = tex;
			}
		}
		ImGui::PopID();

		ImGui::PushID(id++);
		ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		if (ImGui::Button("Remove"))
		{
			component.diffuseMap = nullptr;
		}
		ImGui::PopItemWidth();
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		ImGui::Text(std::string(component.diffuseMap ? component.diffuseMap->GetFilepath().string() : "N/A").c_str());
		ImGui::PopItemWidth();

		ImGui::Text("Specular Map");
		ImGui::PushID(id++);
		if (ImGui::Button("Browse"))
		{
			std::string newTexFilepath = FileDialog::OpenFileDialog("Image Files (*.png, *.jpg, *.jpeg)\0*.png;*.jpg;\0*.jpeg\0");
			if (!newTexFilepath.empty())
			{
				auto tex = Texture2D::Create(newTexFilepath, false, Texture2DDesc());
				component.specularMap = tex;
			}
		}
		ImGui::PopID();

		ImGui::PushID(id++);
		ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		if (ImGui::Button("Remove"))
		{
			component.specularMap = nullptr;
		}
		ImGui::PopItemWidth();
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		ImGui::Text(std::string(component.specularMap ? component.specularMap->GetFilepath().string() : "N/A").c_str());
		ImGui::PopItemWidth();


		ImGui::ColorEdit3("Ambient Color", &component.ambientCol.x);
		ImGui::ColorEdit3("Diffuse Color", &component.diffuseCol.x);
		ImGui::ColorEdit3("Specular Color", &component.specularCol.x);
		ImGui::DragFloat2("Tiling", &component.tiling.x);
		ImGui::DragFloat("Shininess", &component.shininess, s_dragSpeed);
	});

	DrawComponent<DirectionalLightComponent>("Directional Light", entity, [](auto& component)
	{
		ImGui::ColorEdit3("Color", &component.color.x);
		ImGui::DragFloat("Ambient intensity", &component.ambientIntensity, s_dragSpeed);
		ImGui::DragFloat("Diffuse intensity", &component.diffuseIntensity, s_dragSpeed);
		ImGui::DragFloat("Specular intensity", &component.specularIntensity, s_dragSpeed);
	});

	DrawComponent<PointLightComponent>("Point Light", entity, [](auto& component)
	{
		ImGui::ColorEdit3("Color", &component.color.x);
		ImGui::DragFloat("Ambient intensity", &component.ambientIntensity, s_dragSpeed);
		ImGui::DragFloat("Diffuse intensity", &component.diffuseIntensity, s_dragSpeed);
		ImGui::DragFloat("Specular intensity", &component.specularIntensity, s_dragSpeed);
		ImGui::DragFloat("Constant", &component.constant, s_dragSpeed);
		ImGui::DragFloat("Linear", &component.linear, s_dragSpeed);
		ImGui::DragFloat("Quadratic", &component.quadratic, s_dragSpeed);
	});

	DrawComponent<SpotLightComponent>("Spot Light", entity, [](auto& component)
	{
		ImGui::ColorEdit3("Color", &component.color.x);
		ImGui::DragFloat("Ambient intensity", &component.ambientIntensity, s_dragSpeed);
		ImGui::DragFloat("Diffuse intensity", &component.diffuseIntensity, s_dragSpeed);
		ImGui::DragFloat("Specular intensity", &component.specularIntensity, s_dragSpeed);
		ImGui::DragFloat("Constant", &component.constant, s_dragSpeed);
		ImGui::DragFloat("Linear", &component.linear, s_dragSpeed);
		ImGui::DragFloat("Quadratic", &component.quadratic, s_dragSpeed);
		ImGui::DragFloat("Inner cutoff angle", &component.innerCutOffAngle, s_dragSpeed);
		ImGui::DragFloat("Outer cutoff angle", &component.outerCutOffAngle, s_dragSpeed);
	});
}