#pragma once

#include "core/ecs/component_storage.h"
#include "core/ecs/entity.h"
//#include "core/ecs/movement_system.h"

#include <cstdint>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>


namespace neon::gameplay
{
	class World
	{
		using Entity = neon::core::ecs::Entity;
		//using ComponentManager = neon::core::ecs::ComponentManager;
		//using MovementSystem = neon::core::ecs::MovementSystem;
	public:
		// ===== Types =====
		// ===== Constructors =====
		// ===== World API
		void update(double dt) {
			updateSystems(componentManager, dt);
		}
		// ===== Entity API =====
		// ===== Component API =====

		Entity createEntity()
		{
			Entity entity;

			// 优先从空闲列表中回收实体ID
			if (!freeEntities.empty())
			{
				entity = freeEntities.back();
				freeEntities.pop_back();
			}
			else
			{
				// 没有空闲ID，创建新ID
				entity = static_cast<Entity>(entityAlive.size());
				entityAlive.push_back(1);  // 新实体标记为存活
				entityCount++;
				return entity;
			}

			// 复用的实体ID，标记为存活
			entityAlive[static_cast<uint32_t>(entity)] = 1;
			entityCount++;
			return entity;
		}

		void destroyEntity(Entity entity)
		{
			// 检查实体是否有效
			if (!isEntityAlive(entity))
			{
				return;  // 实体已销毁或无效
			}

			// 标记实体为死亡状态
			entityAlive[static_cast<uint32_t>(entity)] = 0;
			entityCount--;

			// 添加到空闲列表以便复用
			freeEntities.push_back(entity);

			// 移除该实体的所有组件（后续实现）
			// removeAllComponents(entity);
		}
		//ComponentManager& getComponentManager() { return componentManager; }
				// 检查实体是否存活
		bool isEntityAlive(Entity entity) const
		{
			uint32_t index = static_cast<uint32_t>(entity);
			// 检查索引范围
			if (index >= entityAlive.size())
			{
				return false;
			}
			return entityAlive[index] != 0;
		}

		// 获取当前存活的实体数量
		size_t getEntityCount() const { return entityCount; }

	private:
		// ===== Types =====

		// ===== Systems =====
		ComponentManager componentManager;
		//系统会调用默认初始化函数
		std::tuple<
			//MovementSystem

		>systems;
		// ===== Entity =====
		std::vector<char> entityAlive;
		std::vector<Entity> freeEntities;
		// ===== Entity Storage =====
		// ===== Stats =====
		size_t entityCount = 0;

		// ===== Internal =====


		// ===== Component API =====
		//updateComponent(componentManager, dt, component...);

		void updateSystems(ComponentManager& cm, double dt)
		{
			std::apply(
				[&](auto&... sys)
				{
					(sys.update(cm, dt), ...);
				},
				systems
			);
		}


	};

}