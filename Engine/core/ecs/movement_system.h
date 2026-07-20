#pragma once

#include "component_storage.h"
#include "ecs_system.h"

namespace neon::core::ecs
{
	struct MovementUpdater {
		void operator()(ComponentManager&, Entity entity, double dt) noexcept;
	};

	//using MovementSystem = EcsSystem<MovementUpdater>;
}