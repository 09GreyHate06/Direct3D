#pragma once
#include <entt/entt.hpp>
#include "Scene.h"
#include "D3DCore/Core/Base.h"

namespace d3dcore
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene)
			: m_handle(handle), m_scene(scene)
		{
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			D3DC_CORE_ASSERT(!HasComponent<T>(), "Component already exists!");
			T& component = m_scene->m_registry.emplace<T>(m_handle, std::forward<Args>(args)...);
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			D3DC_CORE_ASSERT(HasComponent<T>(), "Component does not exists!");
			return m_scene->m_registry.get<T>(m_handle);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_scene->m_registry.any_of<T>(m_handle);
		}

		template<typename T>
		bool RemoveComponent()
		{
			D3DC_CORE_ASSERT(HasComponent<T>(), "Component does not exist!");
			return m_scene->m_registry.remove<T>(m_handle);
		}

		operator bool() const
		{
			return m_handle != entt::null && m_scene->m_registry.valid(m_handle);
		}

		operator entt::entity() const { return m_handle; }
		operator uint32_t() const { return static_cast<uint32_t>(m_handle); }

		bool operator==(const Entity& rhs) const
		{
			return m_handle == rhs.m_handle && m_scene == rhs.m_scene;
		}

		bool operator!=(const Entity& rhs) const
		{
			return !(*this == rhs);
		}

		static bool	SameScene(const Entity& a, const Entity& b)
		{
			return a.m_scene == b.m_scene;
		}

	private:
		entt::entity m_handle = entt::null;
		Scene* m_scene;
	};
}