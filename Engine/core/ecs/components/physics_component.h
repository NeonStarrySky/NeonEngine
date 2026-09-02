#pragma once

#include <glm/glm.hpp>

namespace neon::core::ecs
{

	struct PhysicsComponent
	{
		glm::vec3 position;
		glm::vec3 velocity;
	};
}