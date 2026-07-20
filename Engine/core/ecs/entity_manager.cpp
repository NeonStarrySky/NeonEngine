#include "entity.h"
#include "entity_manager.h"
namespace neon::core::ecs {
	Entity EntityManager::createEntity()
	{
		//没有空余编号
		if (freeEntityId.empty()) {
			return usedEntityId++;
		}
		//有空余编号
		else {
			auto temp = freeEntityId.back();
			freeEntityId.pop_back();
			return temp;
		}
	}
	void EntityManager::destroyEntity(Entity entity)
	{
		freeEntityId.push_back(entity);
	}
}

