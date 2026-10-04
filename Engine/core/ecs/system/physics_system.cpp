#include "physics_system.h"

#include <cmath>

namespace neon::core::ecs
{
	// 两两引力：每一对粒子之间都有吸引力（牛顿引力 + 软化长度，避免 r→0 时力发散）。
	// 总质量归一化为 1（每颗粒子质量 = 1/粒子数），这样调整实体数量时运动尺度基本不变。
	void PhysicsSystem::applyGravity(float deltaTime)
	{
		const std::size_t particleCount = physicsComponents.getEntities().size();
		if (particleCount < 2) return;

		const float particleMass = 1.0f / static_cast<float>(particleCount);
		const float softeningSq = setting.gravitySoftening * setting.gravitySoftening;
		// 每对的公共系数：G * m * dt，后面再乘 1 / (r² + ε²)^{3/2}
		const float pairFactor = setting.gravity * particleMass * deltaTime;

		// 组件是连续存放的，所以用迭代器两两遍历即可，不需要额外的容器或分配
		for (auto itA = physicsComponents.begin(); itA != physicsComponents.end(); ++itA)
		{
			auto&& particleA = (*itA).second;

			auto itB = itA;
			for (++itB; itB != physicsComponents.end(); ++itB)
			{
				auto&& particleB = (*itB).second;

				const glm::vec3 offset = particleB.position - particleA.position;
				const float distanceSq = glm::dot(offset, offset) + softeningSq;
				const float inverseDistance = 1.0f / std::sqrt(distanceSq);
				// a = G * m * r_vec / (r² + ε²)^{3/2}
				const float scale = pairFactor * inverseDistance * inverseDistance * inverseDistance;

				particleA.velocity += offset * scale;
				particleB.velocity -= offset * scale; // 牛顿第三定律：一对粒子受力等大反向
			}
		}
	}

	void PhysicsSystem::update() {
		const float deltaTime = 1.0f / static_cast<float>(setting.FrameRate);

		// 先由两两引力更新速度，再按速度积分位置
		applyGravity(deltaTime);

		for (auto&& componentEntry : physicsComponents) {
			auto& physicsComponent = componentEntry.second;
			auto& position = physicsComponent.position;
			auto& velocity = physicsComponent.velocity;

			// Update the position based on velocity and delta time
			//边界反弹setting里的窗口边界大小
			position += velocity * deltaTime;

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
