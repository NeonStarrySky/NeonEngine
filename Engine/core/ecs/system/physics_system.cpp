#include "physics_system.h"

namespace neon::core::ecs
{
	void PhysicsSystem::update() {
		for (auto&& componentEntry : physicsComponents) {
			auto& physicsComponent = componentEntry.second;
			auto& position = physicsComponent.position;
			auto& velocity = physicsComponent.velocity;

			// Update the position based on velocity and delta time
			//边界反弹setting里的窗口边界大小
			position += velocity * (1.0f / setting.FrameRate); // Assuming a fixed delta time of 16ms (60 FPS)

			if (position.x < -1.0f) {
				position.x = -1.0f;
				if (velocity.x < 0.0f) velocity.x = -velocity.x;
			} else if (position.x > 1.0f) {
				position.x = 1.0f;
				if (velocity.x > 0.0f) velocity.x = -velocity.x;
			}

			if (position.y < -1.0f) {
				position.y = -1.0f;
				if (velocity.y < 0.0f) velocity.y = -velocity.y;
			} else if (position.y > 1.0f) {
				position.y = 1.0f;
				if (velocity.y > 0.0f) velocity.y = -velocity.y;
			}
		}
	}
}

