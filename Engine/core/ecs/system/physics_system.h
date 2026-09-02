#pragma once

#include "core/ecs/component_manager.h"
#include "core/ecs/component_storage.h"
#include "core/ecs/components/physics_component.h"
#include "core/logger.h"
#include"core/setting.h"

namespace neon::core::ecs
{

	class PhysicsSystem
	{
		Logger& logger;
		Setting& setting;
		ComponentManager& componentManager;
		ComponentStorage<PhysicsComponent>& physicsComponents;
	public:
		PhysicsSystem(Logger& logger, Setting& setting, ComponentManager& componentManager) :
			logger(logger),
			setting(setting),
			componentManager(componentManager),
			physicsComponents(componentManager.registerComponent<PhysicsComponent>())
		{

		}
		void update();
	};
}