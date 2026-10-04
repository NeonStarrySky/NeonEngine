#pragma once

#include "core/ecs/component_manager.h"
#include "core/ecs/component_storage.h"
#include "core/ecs/components/physics_component.h"
#include "core/logger.h"
#include"core/setting.h"

namespace neon::core::ecs
{

	class PhysicsSystem
	{
		Logger& logger;
		Setting& setting;
		ComponentManager& componentManager;
		ComponentStorage<PhysicsComponent>& physicsComponents;

		// 两两引力：任意两个粒子之间都存在吸引力（O(N²)，实体数量多时开销明显）
		void applyGravity(float deltaTime);
	public:
		// 当前所有粒子的总动能（与引力一致，每颗粒子质量按 1/粒子数 归一化）
		float getTotalKineticEnergy();
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
