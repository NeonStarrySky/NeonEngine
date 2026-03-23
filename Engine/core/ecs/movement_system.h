#pragma once

#include "ecs_components.h"
#include "ecs_system.h"

// removed include of world.h to avoid circular include; ecs_system.h forward-declares World

namespace neon::core::ecs
{
	struct MovementUpdater {
		void operator()(ComponentManager&, Entity entity, double dt) noexcept;
	};

	using MovementSystem = EcsSystem<MovementUpdater>;
}