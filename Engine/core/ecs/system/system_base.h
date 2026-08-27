#pragma once

#include "core/ecs/component_manager.h"
#include "core/logger.h"

namespace neon::core::ecs
{
	class system_base
	{
		//
	protected:
		neon::core::Logger& logger;
		neon::core::ecs::ComponentManager& componentManager;
	}
}