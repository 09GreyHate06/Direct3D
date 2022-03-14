#include "d3dcpch.h"
#include "Scene.h"
#include "Entity.h"
#include "Components.h"

namespace d3dcore
{
	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntity(name, Entity());
	}

	Entity Scene::CreateEntity(const std::string& name, Entity parent)
	{
		Entity entity = { m_registry.create(), this };
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<TagComponent>().tag = name.empty() ? "Entity" : name;
		entity.AddComponent<RelationshipComponent>();

		SetEntityRelationship(entity, parent);

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		SetEntityRelationship(entity, Entity());

		DestroyEntityChildren(entity);
		m_registry.destroy(entity);
	}

	void Scene::SetEntityRelationship(Entity entity, Entity parent)
	{
		auto& relationship = entity.GetComponent<RelationshipComponent>();

		if (relationship.parent)
		{
			if (relationship.prev && relationship.next)
			{
				relationship.prev.GetComponent<RelationshipComponent>().next = relationship.next;
				relationship.next.GetComponent<RelationshipComponent>().prev = relationship.prev;

				relationship.next = Entity();
				relationship.prev = Entity();

			}
			else if (relationship.next)
			{
				relationship.parent.GetComponent<RelationshipComponent>().first = relationship.next;
				relationship.next.GetComponent<RelationshipComponent>().prev = Entity();
				relationship.next = Entity();
			}
			else if (relationship.prev)
			{
				relationship.prev.GetComponent<RelationshipComponent>().next = Entity();
				relationship.prev = Entity();
			}
			else
				relationship.parent.GetComponent<RelationshipComponent>().first = Entity();

			relationship.parent = Entity();
		}

		if (parent)
		{
			D3DC_CORE_ASSERT(Entity::SameScene(entity, parent), "Parent is not in the same scene");

			auto& parentRelationshp = parent.GetComponent<RelationshipComponent>();
			if (!parentRelationshp.first)
			{
				parentRelationshp.first = entity;
			}
			else
			{
				Entity prev;
				Entity current = parentRelationshp.first;
				while (current)
				{
					prev = current;
					current = current.GetComponent<RelationshipComponent>().next;
				}
				prev.GetComponent<RelationshipComponent>().next = entity;
				relationship.prev = prev;
			}

			relationship.parent = parent;
		}
	}

	void Scene::DestroyEntityChildren(Entity entity)
	{
		auto& relationship = entity.GetComponent<RelationshipComponent>();
		Entity next;
		Entity current = relationship.first;
		while (current)
		{
			auto& currentRelationshipComponent = current.GetComponent<RelationshipComponent>();

			if (currentRelationshipComponent.first)
				DestroyEntityChildren(current);

			next = currentRelationshipComponent.next;
			DestroyEntity(current);
			current = next;
		}
	}

	bool Scene::IsValid(Entity entity)
	{
		return m_registry.valid(entity);
	}
}

