#pragma once

#include "core/ecs/component_manager.h"
#include "core/ecs/component_storage.h"
#include "core/ecs/components/physics_component.h"
#include "core/logger.h"
#include"core/setting.h"

#include <cstddef>

namespace neon::core::ecs
{
	/// @brief 单步的斥力诊断量，用来判断动能增长是否与“斥力生效”同步。
	struct RepulsionStats
	{
		float velocityChange = 0.0f;		// 这一步斥力给出的速度改变量 Σ|Δv|
		float potentialEnergy = 0.0f;		// 斥力总势能（未钳制的解析形式；有钳制时它是上界）
		std::size_t pairCount = 0;			// 距离小于斥力半径的对数（斥力真正生效）
		std::size_t clampedPairCount = 0;	// 其中触发加速度上限的对数
	};

	class PhysicsSystem
	{
		Logger& logger;
		Setting& setting;
		ComponentManager& componentManager;
		ComponentStorage<PhysicsComponent>& physicsComponents;
		RepulsionStats lastRepulsion;		// 上一步的斥力诊断量

		// 引力 + 近距斥力：任意两个粒子之间都有相互作用（O(N²)，实体数量多时开销明显）
		void applyGravity(float deltaTime);
	public:
		// 当前所有粒子的总动能（与引力一致，每颗粒子质量按 1/粒子数 归一化）
		float getTotalKineticEnergy();
		// 引力总势能（软化形式，与 applyGravity 使用的力一致）：用于检查总能量是否守恒
		float getTotalPotentialEnergy();
		// 上一步的斥力诊断量
		const RepulsionStats& getLastRepulsionStats() const { return lastRepulsion; }
		PhysicsSystem(Logger& logger, Setting& setting, ComponentManager& componentManager) :
			logger(logger),
			setting(setting),
			componentManager(componentManager),
			physicsComponents(componentManager.registerComponent<PhysicsComponent>())
		{

		}
		void update();
	};
}
