#pragma once

#include "core/ecs/component_manager.h"
#include "core/logger.h"

namespace neon::core::ecs
{
	class SystemBase
	{
		Logger& logger;
		ComponentManager& componentManager;
	public:
		SystemBase(ComponentManager& componentManager, Logger& logger) :
			componentManager(componentManager),
			logger(logger)
		{}
		void update();
	};
}