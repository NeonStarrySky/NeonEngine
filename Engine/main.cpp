#include "core/engine.h"

#include "core/ecs/component_manager.h"
#include "core/ecs/component_storage.h"
#include "core/ecs/entity.h"
#include "core/ecs/entity_manager.h"
#include "graphics/gl/shader_manager.h"


#include "core/ecs/system/physics_system.h"
#include "core/ecs/system/sprite_system.h"

#include "graphics/frameRateController.h"
#include "graphics/gl/mesh.h"
#include "graphics/gl/renderer.h"
#include "graphics/gl/window.h"
#include "graphics/texture_manager.h"

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
		neon::core::ecs::PhysicsSystem, neon::graphics::FrameRateController,
		neon::graphics::gl::Window, neon::graphics::Renderer,
		neon::core::ecs::SpriteSystem, neon::graphics::gl::Mesh,
		neon::graphics::gl::TextureManager, neon::graphics::gl::Texture,
		neon::graphics::gl::Vertex, neon::core::ecs::SpriteRegion,
		neon::graphics::gl::ShaderManager, neon::graphics::gl::Program,
		neon::graphics::gl::Shader, neon::graphics::WindowInfo;

	std::system("chcp 65001 > nul");  // 65001 就是 UTF-8

	/*申明*/
	//common
	Logger logger{};
	Setting setting;

	/*创建*/
	//ecs
	EntityManager entityManager{ logger };
	ComponentManager componentManager;
	TextureManager textureManager{ logger };
	ShaderManager shaderManager{ logger };

	PhysicsSystem physicsSystem(logger, setting, componentManager);
	SpriteSystem spriteSystem(logger, setting, componentManager);

	auto&& entity = entityManager.createEntity();

	auto&& physicsStorage = componentManager.get<core::ecs::PhysicsComponent>();
	auto&& spriteStorage = componentManager.get<core::ecs::SpriteComponent>();

	//graphics

	{
		auto temp_window = graphics::gl::initGlad(Window::GLInfo(), logger);
	}

	Window window{ WindowInfo{ 1920, 1080, "Neon Engine" }, logger };
	window.makeContextCurrent();

	FrameRateController frameRateController;
	frameRateController.setFrameRate(144);
	Renderer renderer;


	/*gameplay*/
	//graphics
	Program program;
	try {
		textureManager.loadTexture("assets/images/pic.png");

		shaderManager.init(); // Initialize the shader manager
		shaderManager.loadShaders({// Load and compile shaders from specified file paths and types
			{"assets/shaders/vertex_shader.glsl", GL_VERTEX_SHADER},
			{ "assets/shaders/fragment_shader.glsl", GL_FRAGMENT_SHADER }
			});
		shaderManager.linkPrograms({ "assets/shaders/vertex_shader.glsl", "assets/shaders/fragment_shader.glsl" });


		program = shaderManager.buildShader();
	}
	catch (const std::runtime_error& e)
	{
		logger.error("Shader build error: {}", e.what());
		return EXIT_FAILURE;
	}


	//ecs
	physicsStorage.addTo(
		entity,
		core::ecs::PhysicsComponent{
			.position = glm::vec3{0, 0, 0},
			.velocity = glm::vec3{0.5, 0.2, 0}
		}
	);


	std::vector<Vertex> vertices = {
	{ {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f} },  // 左下
	{ { 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} },  // 右下
	{ { 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} },  // 右上
	{ {-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} }   // 左上
	};

	// 两个三角形，逆时针（CCW）为正面
	std::vector<unsigned int> indices = {
		0, 1, 2,   // 第一个三角形：左下 → 右下 → 右上
		0, 2, 3    // 第二个三角形：左下 → 右上 → 左上
	};
	Mesh mesh{ vertices, indices };


	auto texture = textureManager.loadTexture("assets/images/pic.png").lock();
	if (!texture) {
		logger.error("Failed to load texture.");
		return EXIT_FAILURE;
	}

	SpriteRegion region{ 0, 0, texture->info().width, texture->info().height };

	spriteStorage.addTo(
		entity,
		core::ecs::SpriteComponent{ mesh, *texture, region }
	);

	auto&& spriteComponent = spriteStorage.get(entity);
	auto&& physicsComponent = physicsStorage.get(entity);

	/*game loop*/
	while (!glfwWindowShouldClose(window.getGLFWwindow()))
	{

		window.pollEvents();



		//ecs
		physicsSystem.update();

		//for (auto& component : physicsStorage) {
		//	std::cout << component << std::endl;
		//}

		//graphics
		// 清屏
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderer.Draw(
			spriteComponent,
			physicsComponent,
			program,
			glm::mat4(1.0f),
			glm::mat4(1.0f)
		);

		window.swapBuffers();

		frameRateController.checkAndWait();
	}

}