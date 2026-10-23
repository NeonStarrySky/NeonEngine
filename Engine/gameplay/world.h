#pragma once

#include "core/ecs/dynamic_component_manager.h"
#include "core/ecs/entity_manager.h"
#include "core/ecs/system/physics_system.h"
#include "core/ecs/system/sprite_system.h"

#include <cstddef>

namespace neon::gameplay
{
	class World
	{
		using Entity = core::ecs::Entity;

		core::ecs::EntityManager entityManager;
		core::ecs::DynamicComponentManager dynamicComponentManager;
		// Systems borrow the component manager and must be destroyed before it.
		core::ecs::PhysicsSystem physicsSystem;
		core::ecs::SpriteSystem spriteSystem;

	public:
		World(core::Logger& logger, core::Setting& setting) :
			entityManager(logger),
			physicsSystem(logger, setting, dynamicComponentManager),
			spriteSystem(logger, setting, dynamicComponentManager)
		{}

		World(const World&) = delete;
		World& operator=(const World&) = delete;
		World(World&&) = delete;
		World& operator=(World&&) = delete;

		void update()
		{
			physicsSystem.update();
			spriteSystem.update();
		}

		Entity createEntity()
		{
			return entityManager.createEntity();
		}

		void destroyEntity(Entity entity)
		{
			if (!entityManager.isValid(entity)) {
				return;
			}
			dynamicComponentManager.removeAllComponents(entity);
			entityManager.destroyEntity(entity);
		}

		bool isEntityAlive(Entity entity) const
		{
			return entityManager.isValid(entity);
		}

		std::size_t getEntityCount() const
		{
			return entityManager.getEntityCount();
		}

		core::ecs::DynamicComponentManager& getDynamicComponentManager()
		{
			return dynamicComponentManager;
		}
	};
}
