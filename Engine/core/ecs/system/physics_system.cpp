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
		RepulsionStats stepRepulsion;

		const std::size_t particleCount = physicsComponents.getEntities().size();
		if (particleCount < 2)
		{
			return;
		}

		const float particleMass = 1.0f / static_cast<float>(particleCount);
		const float softeningSq = setting.gravitySoftening * setting.gravitySoftening;
		// 每对的公共系数：G * m * dt，后面再乘 1 / (r² + ε²)^{3/2}
		const float pairFactor = setting.gravity * particleMass * deltaTime;

		const float repulsionRadiusSq = setting.repulsionRadius * setting.repulsionRadius;
		const float inverseCutoffThird = repulsionRadiusSq > 0.0f
			? 1.0f / (repulsionRadiusSq * setting.repulsionRadius)
			: 0.0f;
		const float inverseCutoffFourth = repulsionRadiusSq > 0.0f
			? 1.0f / (repulsionRadiusSq * repulsionRadiusSq)
			: 0.0f;
		const float repulsionFactor = setting.repulsionStrength * particleMass * deltaTime;
		// 斥力势能的系数（与下面使用的力严格对应）：U = k * (1/(3r³) + r/r₀⁴ - 4/(3r₀³))，在 r = r₀ 处为 0
		const float repulsionPotentialFactor = setting.repulsionStrength * particleMass * particleMass;
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
					const bool clamped = repulsionMagnitude > maxRepulsionAcceleration;
					if (clamped) repulsionMagnitude = maxRepulsionAcceleration;

					acceleration -= offset * repulsionMagnitude; // 方向与引力相反：把两颗推开

					// 诊断：本子步斥力贡献的速度改变量 |Δv| = magnitude * |offset|、生效对数与斥力势能
					const float distance = std::sqrt(rawDistanceSq);
					const float inverseDistanceCubed = inverseDistanceSq * inverseDistance;
					++stepRepulsion.pairCount;
					if (clamped) ++stepRepulsion.clampedPairCount;
					stepRepulsion.velocityChange += repulsionMagnitude * distance;
					stepRepulsion.potentialEnergy += repulsionPotentialFactor *
						(inverseDistanceCubed / 3.0f + distance * inverseCutoffFourth - 4.0f * inverseCutoffThird / 3.0f);
				}

				particleA.velocity += acceleration;
				particleB.velocity -= acceleration; // 牛顿第三定律：一对粒子受力等大反向
			}
		}

		// 累加到本帧的诊断量上（一帧可能有多个子步）
		lastRepulsion.velocityChange += stepRepulsion.velocityChange;
		lastRepulsion.potentialEnergy += stepRepulsion.potentialEnergy;
		lastRepulsion.pairCount = stepRepulsion.pairCount;
		lastRepulsion.clampedPairCount = stepRepulsion.clampedPairCount;
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

	// 引力势能：U = -Σ_{i<j} G m² / sqrt(r² + ε²)，与 applyGravity 里的软化力严格对应
	float PhysicsSystem::getTotalPotentialEnergy()
	{
		const std::vector<PhysicsComponent>& particles = physicsComponents.getAllComponents();
		const std::size_t particleCount = particles.size();
		if (particleCount < 2) return 0.0f;

		const float particleMass = 1.0f / static_cast<float>(particleCount);
		const float softeningSq = setting.gravitySoftening * setting.gravitySoftening;
		const float pairFactor = -setting.gravity * particleMass * particleMass;

		float totalPotentialEnergy = 0.0f;
		for (std::size_t i = 0; i < particleCount; ++i)
		{
			for (std::size_t j = i + 1; j < particleCount; ++j)
			{
				const glm::vec3 offset = particles[j].position - particles[i].position;
				const float distanceSq = glm::dot(offset, offset) + softeningSq;
				totalPotentialEnergy += pairFactor / std::sqrt(distanceSq);
			}
		}
		return totalPotentialEnergy;
	}

	void PhysicsSystem::initializeBoundary()
	{
		// 边界在启动时确认一次。之后没有 setter，这里的重复调用也不会切换边界，
		// 因为“边界种类”属于启动配置：运行期间换边界会让粒子突然被另一套规则处理。
		if (boundary)
		{
			logger.warn(
				"Physics: boundary is already '{}' and cannot be switched at runtime; ignoring the request for '{}'.",
				boundary->getName(),
				physics::boundaryTypeName(setting.boundary)
			);
			return;
		}

		boundary = physics::createBoundary(setting.boundary, physics::defaultHalfExtent);
		logger.info(
			"Physics: boundary = '{}', visible area = [{}, {}]^2 (fixed at startup).",
			boundary->getName(),
			-boundary->getHalfExtent(),
			boundary->getHalfExtent()
		);
	}

	void PhysicsSystem::update(float frameDeltaTime) {
		// 边界应当由启动流程确认；万一没有，就在这里补上（仍然只会确认一次）
		if (!boundary) initializeBoundary();

		// 帧时长保护：太小会让子步数变成 0，太大（例如卡顿之后）会让一帧里塞进过多子步
		constexpr float minFrameDelta = 1.0f / 2000.0f;
		constexpr float maxFrameDelta = 0.1f;
		constexpr int maxSubsteps = 32;

		float frameDelta = frameDeltaTime;
		if (frameDelta < minFrameDelta) frameDelta = minFrameDelta;
		if (frameDelta > maxFrameDelta) frameDelta = maxFrameDelta;

		// 子步积分：把这一帧的相互作用与积分拆成若干小步，使内部积分步长 ≈ 1/physicsRate。
		// 刚性斥力（1/r⁴）在 dt = 1/60 下显式积分会指数式注入能量，细化步长即可消除这种不稳定；
		// 关键在于每帧推进的时间仍然等于真实帧时长，所以提高显示帧率不会让模拟变慢。
		int substeps = static_cast<int>(std::lround(frameDelta * setting.physicsRate));
		if (substeps < 1) substeps = 1;
		if (substeps > maxSubsteps) substeps = maxSubsteps;
		lastSubsteps = substeps;

		const float deltaTime = frameDelta / static_cast<float>(substeps);
		simulatedTime += frameDelta;

		lastRepulsion = RepulsionStats{};
		for (int substep = 0; substep < substeps; ++substep) {
			// 先由粒子间相互作用（引力 + 近距斥力）更新速度，再按速度积分位置
			applyGravity(deltaTime);

			for (auto&& componentEntry : physicsComponents) {
				auto& physicsComponent = componentEntry.second;
				auto& position = physicsComponent.position;
				auto& velocity = physicsComponent.velocity;

				// Update the position based on velocity and delta time
				position += velocity * deltaTime;

				// 到达边界的粒子交给边界处理（循环 / 反弹），物理系统不需要知道是哪一种
				totalBoundaryHits += static_cast<std::size_t>(boundary->apply(position, velocity));
			}
		}
	}
}
