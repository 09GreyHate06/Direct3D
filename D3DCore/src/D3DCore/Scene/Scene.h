#pragma once
#include <entt/entt.hpp>

namespace d3dcore
{
	class Entity;

	class Scene
	{
	public:
		Scene() = default;
		virtual ~Scene();

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntity(const std::string& name, Entity parent);
		void DestroyEntity(Entity entity);

		void SetEntityRelationship(Entity entity, Entity parent);
		void DestroyEntityChildren(Entity entity);

		bool IsValid(Entity entity);
		const entt::registry& GetRegistry() const { return m_registry; }

	private:
		entt::registry m_registry;

		friend class Entity;
		friend class System;
	};
}