#include "physics_system.h"

namespace neon::core::ecs
{
	void PhysicsSystem::update() {
		for (auto& [position, velocity] : physicsComponents) {
			// Update the position based on velocity and delta time
			position += velocity * (1.0f / setting.FrameRate); // Assuming a fixed delta time of 16ms (60 FPS)
		}
	}
}

