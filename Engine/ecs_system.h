#pragma once

#include "ecs_components.h"
#include "entity.h"

#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

namespace neon::core::ecs
{
	namespace ecs_system_
	{
		template <typename D>
		concept updater =
			std::is_nothrow_invocable_v<D, ComponentManager&, Entity, double>;
	}

	template <ecs_system_::updater update_>
	class EcsSystem
	{
		//Sparse Set
		std::vector<Entity> entities;
		std::vector <Entity> entityToIndex;
	public:
		void addEntity(Entity e)
		{
			if (e >= entityToIndex.size())
				entityToIndex.resize(e + 1, -1);

			entities.push_back(e);
			entityToIndex[e] = entities.size() - 1;
		}
		void removeEntity(Entity e)
		{
			int idx = entityToIndex[e];
			Entity last = entities.back();

			entities[idx] = last;
			entityToIndex[last] = idx;

			entities.pop_back();
			entityToIndex[e] = -1;
		}

		void update(ComponentManager& componentManager, double dt)
		{
			for (auto entity : entities)
			{
				update_{}(componentManager, entity, dt);
			}
		}
	};
}