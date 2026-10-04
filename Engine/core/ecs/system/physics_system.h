#pragma once

#include "core/ecs/component_manager.h"
#include "core/ecs/component_storage.h"
#include "core/ecs/components/physics_component.h"
#include "core/logger.h"
#include "core/physics/boundary.h"
#include"core/setting.h"

#include <cstddef>
#include <memory>

namespace neon::core::ecs
{
	/// @brief 一帧内的斥力诊断量，用来判断动能增长是否与“斥力生效”同步。
	///        velocityChange / potentialEnergy 在一帧的所有子步上累加；pairCount / clampedPairCount 取最后一个子步的快照。
	struct RepulsionStats
	{
		float velocityChange = 0.0f;		// 斥力给出的速度改变量 Σ|Δv|
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
		RepulsionStats lastRepulsion;		// 上一帧的斥力诊断量
		int lastSubsteps = 1;				// 上一帧实际使用的子步数
		std::size_t totalBoundaryHits = 0;	// 累计被边界处理的坐标分量个数（只增不减）
		float simulatedTime = 0.0f;			// 累计推进的模拟时间（秒）

		// 边界：启动时根据 Setting::boundary 创建一次，之后不变（没有 setter，重复初始化会被拒绝）
		std::unique_ptr<physics::Boundary> boundary;

		// 引力 + 近距斥力：任意两个粒子之间都有相互作用（O(N²)，实体数量多时开销明显）
		void applyGravity(float deltaTime);
	public:
		// 当前所有粒子的总动能（与引力一致，每颗粒子质量按 1/粒子数 归一化）
		float getTotalKineticEnergy();
		// 引力总势能（软化形式，与 applyGravity 使用的力一致）：用于检查总能量是否守恒
		float getTotalPotentialEnergy();
		// 上一帧的斥力诊断量
		const RepulsionStats& getLastRepulsionStats() const { return lastRepulsion; }
		// 上一帧实际使用的子步数
		int getLastSubsteps() const { return lastSubsteps; }
		// 累计被边界处理的坐标分量个数（0 表示还没有粒子碰到边界）
		std::size_t getTotalBoundaryHits() const { return totalBoundaryHits; }
		// 累计推进的模拟时间（秒）
		float getSimulatedTime() const { return simulatedTime; }
		// 当前生效的边界（启动时确认，未初始化时返回 nullptr）
		const physics::Boundary* getBoundary() const { return boundary.get(); }
		PhysicsSystem(Logger& logger, Setting& setting, ComponentManager& componentManager) :
			logger(logger),
			setting(setting),
			componentManager(componentManager),
			physicsComponents(componentManager.registerComponent<PhysicsComponent>())
		{

		}
		/// @param frameDeltaTime 本帧的真实时长（秒）：子步数由它和 Setting::physicsRate 决定
		void update(float frameDeltaTime);
		/// 确认本系统的边界：只在启动流程里调用一次，之后边界不再改变。
		/// 重复调用不会切换边界，只会打一条警告。
		void initializeBoundary();
	};
}
