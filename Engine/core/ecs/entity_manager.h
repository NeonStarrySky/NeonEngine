#pragma once

#include <vector>

#include "entity.h"

#include "core/logger.h"

namespace neon::core::ecs
{
	class EntityManager
	{
		Logger& logger;

		std::vector<unsigned char> entityAlive;
		std::vector<Entity> freeEntityId;
		std::size_t entityCount = 0;
	public:

		EntityManager(Logger& logger) : logger(logger) {
			logger.info("EntityManager was created.");
		}
		EntityManager& operator=(const EntityManager&) = default;

		Entity createEntity();
		void destroyEntity(Entity entity);
		bool isValid(Entity entity) const;
		std::size_t getEntityCount() const
		{
			return entityCount;
		}
	};
}
