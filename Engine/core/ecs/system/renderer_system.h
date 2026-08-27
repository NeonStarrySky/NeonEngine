#pragma once

#include "core/ecs/component_manager.h"
#include "core/logger.h"

namespace neon::core::ecs
{
	class RendererSystem
	{
		Logger& logger;
	public:
		void update(ComponentManager& componentManager);
	};
}