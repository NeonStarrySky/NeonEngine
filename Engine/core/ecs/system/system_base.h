#pragma once

#include "core/ecs/dynamic_component_manager.h"
#include "core/ecs/dynamic_component_storage.h"
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
		DynamicComponentManager& dynamicComponentManager;
		DynamicComponentStorage<Component>& Components;
	public:
		System(Logger& logger, Setting& setting, DynamicComponentManager& dynamicComponentManager) :
			logger(logger),
			setting(setting),
			dynamicComponentManager(dynamicComponentManager),
			Components(dynamicComponentManager.registerComponent<Component>())
		{

		}
		void update();
	};
}