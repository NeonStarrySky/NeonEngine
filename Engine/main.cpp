#include "core/engine.h"

#include "core/ecs/component_manager.h"
#include"core/ecs/component_storage.h"
#include "core/ecs/entity.h"
#include"core/ecs/entity_manager.h"

#include "core/type_system/type_id.h"

#include <iostream>

int main(int argc, char* argv[]) {
	auto componentManager = neon::core::ecs::ComponentManager();
	auto entityManager = neon::core::ecs::EntityManager();
	auto entity1 = entityManager.createEntity();
	componentManager.registerComponent<neon::core::ecs::PositionComponentStorage>();
	componentManager.add<neon::core::ecs::PositionComponent>(entity1, neon::core::ecs::PositionComponent{ 1.0f, 2.0f, 3.0f });
	neon::core::Engine engine;

	engine.run();

}