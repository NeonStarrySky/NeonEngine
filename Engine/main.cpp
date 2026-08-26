#include "core/engine.h"

#include "core/ecs/component_manager.h"
#include"core/ecs/component_storage.h"
#include "core/ecs/entity.h"
#include"core/ecs/entity_manager.h"

#include "core/type_system/type_id.h"

#include <cstdlib>
#include <iostream>

void outputPositionComponent(const neon::core::ecs::PositionComponent& position) {
	std::cout << "PositionComponent: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
}

int main(int argc, char* argv[]) {
	std::system("chcp 65001 > nul");  // 65001 就是 UTF-8
	auto componentManager = neon::core::ecs::ComponentManager();
	auto entityManager = neon::core::ecs::EntityManager();
	auto entity1 = entityManager.createEntity();
	componentManager.registerComponent<neon::core::ecs::PositionComponentStorage>();
	componentManager.add<neon::core::ecs::PositionComponent>(entity1, neon::core::ecs::PositionComponent{ 1.0f, 2.0f, 3.0f });
	outputPositionComponent(componentManager.get<neon::core::ecs::PositionComponentStorage>().get(entity1));
	neon::core::Engine engine;
	engine.init();
	engine.run();
}