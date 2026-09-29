#include "physics_system.h"

namespace neon::core::ecs
{
	void PhysicsSystem::update() {
		for (auto& [position, velocity] : physicsComponents) {
			// Update the position based on velocity and delta time
			//边界反弹setting里的窗口边界大小
			if (position.x < -1.0f || position.x > 1.0) {
				velocity.x = -velocity.x; // Reverse the x velocity
			}
			if (position.y < -1.0f || position.y > 1.0) {
				velocity.y = -velocity.y; // Reverse the y velocity
			}
			position += velocity * (1.0f / setting.FrameRate); // Assuming a fixed delta time of 16ms (60 FPS)
		}
	}
}

