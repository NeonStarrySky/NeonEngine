#include "component_manager.h"
#include "component_storage.h"
#include "movement_system.h"

#include <glm/detail/type_vec3.hpp>

namespace neon::core::ecs {
	void MovementUpdater::operator()(ComponentManager& componentManager, Entity entity, double dt) noexcept
	{
		//componentManager.get<PositionComponents>()[entity] += componentManager.get<VelocityComponents>()[entity];
	}
}