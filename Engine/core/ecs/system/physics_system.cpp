#include "physics_system.h"

#include <cmath>

namespace neon::core::ecs
{
	// 粒子间相互作用，每一对粒子都计算一次：
	//   1) 引力：任何距离都存在（牛顿引力 + 软化长度，避免 r→0 时发散）；
	//   2) 近距斥力：只有距离小于 Setting::repulsionRadius 时才出现，按 1/r⁴ 增长，
	//      因此贴近时会迅速变得非常强，阻止粒子互相穿透、塌成一个点。
	// 总质量归一化为 1（每颗粒子质量 = 1/粒子数），这样调整实体数量时运动尺度基本不变。
	void PhysicsSystem::applyGravity(float deltaTime)
	{
		const std::size_t particleCount = physicsComponents.getEntities().size();
		if (particleCount < 2) return;

		const float particleMass = 1.0f / static_cast<float>(particleCount);
		const float softeningSq = setting.gravitySoftening * setting.gravitySoftening;
		// 每对的公共系数：G * m * dt，后面再乘 1 / (r² + ε²)^{3/2}
		const float pairFactor = setting.gravity * particleMass * deltaTime;

		const float repulsionRadiusSq = setting.repulsionRadius * setting.repulsionRadius;
		const float inverseCutoffFourth = repulsionRadiusSq > 0.0f
			? 1.0f / (repulsionRadiusSq * repulsionRadiusSq)
			: 0.0f;
		const float repulsionFactor = setting.repulsionStrength * particleMass * deltaTime;
		// 斥力加速度上限：1/r⁴ 在 r→0 时理论上是无穷大，直接积分会把速度打爆（出现 NaN）。
		// 上限取得比较小，相当于“近距防重叠”的局部冲击：太大会把整个凝聚团踢散。
		constexpr float maxRepulsionAcceleration = 5.0f;

		// 组件是连续存放的，所以用迭代器两两遍历即可，不需要额外的容器或分配
		for (auto itA = physicsComponents.begin(); itA != physicsComponents.end(); ++itA)
		{
			auto&& particleA = (*itA).second;

			auto itB = itA;
			for (++itB; itB != physicsComponents.end(); ++itB)
			{
				auto&& particleB = (*itB).second;

				const glm::vec3 offset = particleB.position - particleA.position;
				const float rawDistanceSq = glm::dot(offset, offset);

				// 引力：a = G * m * r_vec / (r² + ε²)^{3/2}
				const float softenedDistanceSq = rawDistanceSq + softeningSq;
				const float inverseSoftened = 1.0f / std::sqrt(softenedDistanceSq);
				const float gravityScale = pairFactor * inverseSoftened * inverseSoftened * inverseSoftened;
				glm::vec3 acceleration = offset * gravityScale;

				// 近距斥力：只在 r < repulsionRadius 时生效
				if (rawDistanceSq < repulsionRadiusSq && rawDistanceSq > 0.0f)
				{
					const float inverseDistance = 1.0f / std::sqrt(rawDistanceSq);
					const float inverseDistanceSq = inverseDistance * inverseDistance;
					const float inverseDistanceFourth = inverseDistanceSq * inverseDistanceSq;
					// (1/r⁴ - 1/r₀⁴)：在 r = r₀ 处平滑归零，避免力的突跳
					float repulsionMagnitude =
						repulsionFactor * (inverseDistanceFourth - inverseCutoffFourth) * inverseDistance;
					if (repulsionMagnitude > maxRepulsionAcceleration) repulsionMagnitude = maxRepulsionAcceleration;

					acceleration -= offset * repulsionMagnitude; // 方向与引力相反：把两颗推开
				}

				particleA.velocity += acceleration;
				particleB.velocity -= acceleration; // 牛顿第三定律：一对粒子受力等大反向
			}
		}
	}

	// 总动能 = Σ ½ m v²；质量与引力一致（每颗 1/粒子数），因此数值也落在同一量级
	float PhysicsSystem::getTotalKineticEnergy()
	{
		const std::vector<PhysicsComponent>& particles = physicsComponents.getAllComponents();
		if (particles.empty()) return 0.0f;

		const float particleMass = 1.0f / static_cast<float>(particles.size());

		float totalKineticEnergy = 0.0f;
		for (const PhysicsComponent& particle : particles)
			totalKineticEnergy += 0.5f * particleMass * glm::dot(particle.velocity, particle.velocity);

		return totalKineticEnergy;
	}

	void PhysicsSystem::update() {
		const float deltaTime = 1.0f / static_cast<float>(setting.FrameRate);

		// 先由粒子间相互作用（引力 + 近距斥力）更新速度，再按速度积分位置
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
