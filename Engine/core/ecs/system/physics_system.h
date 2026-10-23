#pragma once

#include "core/ecs/dynamic_component_manager.h"
#include "core/ecs/dynamic_component_storage.h"
#include "core/ecs/components/physics_component.h"
#include "core/logger.h"
#include"core/setting.h"

namespace neon::core::ecs
{

	class PhysicsSystem
	{
		Logger& logger;
		Setting& setting;
		DynamicComponentManager& dynamicComponentManager;
		DynamicComponentStorage<PhysicsComponent>& physicsComponents;
	public:
		PhysicsSystem(Logger& logger, Setting& setting, DynamicComponentManager& dynamicComponentManager) :
			logger(logger),
			setting(setting),
			dynamicComponentManager(dynamicComponentManager),
			physicsComponents(dynamicComponentManager.registerComponent<PhysicsComponent>())
		{

		}
		void update();
	};
}