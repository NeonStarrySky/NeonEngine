#include "core/engine.h"

#include "core/ecs/component_manager.h"
#include"core/ecs/component_storage.h"
#include "core/ecs/entity.h"
#include"core/ecs/entity_manager.h"
#include "core/ecs/system/physics_system.h"

#include "graphics/frameRateController.h"

#include "core/type_system/type_id.h"

#include <cstdlib>
#include <iostream>
#include <ranges>

// 先给 glm::vec3 重载输出
std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
	os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
	return os;
}

// 再输出 PhysicsComponent
std::ostream& operator<<(std::ostream& os, const neon::core::ecs::PhysicsComponent& p) {
	os << "PhysicsComponent{ .position = " << p.position
		<< ", .velocity = " << p.velocity << " }";
	return os;
}

int main(int argc, char* argv[]) {

	using namespace neon;
	using
		neon::core::Logger, neon::core::Setting,
		neon::core::ecs::EntityManager, neon::core::ecs::ComponentManager,
		neon::core::ecs::PhysicsSystem, neon::graphics::FrameRateController;

	std::system("chcp 65001 > nul");  // 65001 就是 UTF-8

	Logger logger{};
	Setting setting;

	EntityManager entityManager{ logger };
	ComponentManager componentManager;

	PhysicsSystem physicsSystem(logger, setting, componentManager);
	auto&& entity = entityManager.createEntity();
	auto&& physicsStorage = componentManager.get<core::ecs::PhysicsComponent>();

	FrameRateController frameRateController;
	frameRateController.setFrameRate(60);

	physicsStorage.addTo(
		entity,
		core::ecs::PhysicsComponent{
			.position = glm::vec3{0, 0, 0},
			.velocity = glm::vec3{0.1, 0, 0}
		}
	);

	while (true)
	{
		physicsSystem.update();
		frameRateController.checkAndWait();
		for (auto& component : physicsStorage) {
			std::cout << component << std::endl;
		}
	}
	//neon::core::Engine engine;
	//engine.init();
	//engine.run();
}