#pragma once

#include "core/ecs/component_manager.h"
#include "core/ecs/component_storage.h"
#include "core/logger.h"
#include"core/setting.h"

namespace neon::core::ecs
{

	struct Component
	{

	};

	class System
	{
		Logger& logger;
		Setting& setting;
		ComponentManager& componentManager;
		ComponentStorage<Component>& Components;
	public:
		System(Logger& logger, Setting& setting, ComponentManager& componentManager) :
			logger(logger),
			setting(setting),
			componentManager(componentManager),
			Components(componentManager.registerComponent<Component>())
		{

		}
		void update();
	};
}