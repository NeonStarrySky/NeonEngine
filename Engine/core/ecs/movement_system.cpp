#include "ecs_components.h"
#include "ecs_system.h"
#include "movement_system.h"

#include <glm/detail/type_vec3.hpp>

namespace neon::core::ecs {
	void MovementUpdater::operator()(ComponentManager& componentManager, Entity entity, double dt) noexcept
	{
		componentManager.get<PositionComponents>()[entity] += componentManager.get<VelocityComponents>()[entity];
	}
}