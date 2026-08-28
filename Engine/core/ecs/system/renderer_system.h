#pragma once

#include "core/ecs/component_manager.h"
#include "core/logger.h"

namespace neon::core::ecs
{
	class RendererSystem
	{
		Logger& logger;
		ComponentManager& componentManager;
	public:
		RendererSystem(ComponentManager& componentManager, Logger& logger) :
			componentManager(componentManager),
			logger(logger)
		{}
		void update();
	};
}