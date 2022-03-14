#pragma once
#include "Scene.h"

namespace d3dcore
{
	class System
	{
	private:
		Scene* m_scene = nullptr;

	public:
		System(Scene* scene) : m_scene(scene) {}
		System() : System(nullptr) {}
		virtual ~System() = default;

		inline void SetContext(Scene* scene) { m_scene = scene; }

	protected:
		entt::registry& GetSceneRegistry() const { return m_scene->m_registry; }
		Scene* GetScene() const { return m_scene; }
	};
}