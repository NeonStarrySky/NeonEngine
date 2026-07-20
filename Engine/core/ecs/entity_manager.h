#pragma once

#include <vector>

#include "entity.h"

namespace neon::core::ecs
{
	class EntityManager
	{
		Entity usedEntityId = 0;
		std::vector<Entity> freeEntityId;
	public:
		EntityManager() = default;
		Entity createEntity();
		void destroyEntity(Entity entity);
		bool isValid(Entity entity) const;
	private:
		std::vector<Entity> m_entities;
	};
}