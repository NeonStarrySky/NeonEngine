#include "entity.h"
#include "entity_manager.h"
namespace neon::core::ecs {
	Entity EntityManager::createEntity()
	{
		//没有空余编号
		if (freeEntityId.empty()) {
			Entity entity = entityAlive.size();
			entityAlive.push_back(1);
			++entityCount;
			return entity;
		}
		//有空余编号
		else {
			auto temp = freeEntityId.back();
			freeEntityId.pop_back();
			entityAlive[temp] = 1;
			++entityCount;
			return temp;
		}
	}
	void EntityManager::destroyEntity(Entity entity)
	{
		if (!isValid(entity)) {
			return;
		}
		freeEntityId.push_back(entity);
		entityAlive[entity] = 0;
		--entityCount;
	}

	bool EntityManager::isValid(Entity entity) const
	{
		return entity < entityAlive.size() && entityAlive[entity] != 0;
	}
}

