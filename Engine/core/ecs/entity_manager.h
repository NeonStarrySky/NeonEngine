#pragma once

#include <vector>

#include "entity.h"

#include "core/logger.h"

namespace neon::core::ecs
{
	class EntityManager
	{
		Logger& logger;

		Entity usedEntityId = 0;
		std::vector<Entity> freeEntityId;
	public:

		EntityManager(Logger& logger) : logger(logger) {
			logger.info("EntityManager was created.");
		}
		EntityManager& operator=(const EntityManager&) = default;

		Entity createEntity();
		void destroyEntity(Entity entity);
		bool isValid(Entity entity) const;
	private:
		std::vector<Entity> m_entities;
	};
}