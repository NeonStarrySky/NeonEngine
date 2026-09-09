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
#include <random>
#include <ranges>

#include <cmath>
#include <deque>

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
inline std::mt19937& getRng() {
	thread_local std::mt19937 rng{ std::random_device{}() };
	return rng;
}

// ---------- 1. 随机单位向量（球面均匀分布） ----------
inline glm::vec3 randomUnitVector() {
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
	glm::vec3 v;
	do {
		v = glm::vec3(dist(getRng()), dist(getRng()), dist(getRng()));
	} while (glm::dot(v, v) > 1.0f || glm::dot(v, v) < 1e-6f); // 拒绝采样，保证在单位球内且非零
	return glm::normalize(v);
}
int main(int argc, char* argv[]) {
	//std::cout << "Game path:" << argv[0] << '\n';

	int frameRate = 144; // 设置目标帧率为 144 FPS
	std::string picPath = "pic.png";
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

	logger.info("Game path: {}", argv[0]);
	/*创建*/
	//ecs
	EntityManager entityManager{ logger };
	ComponentManager componentManager;
	TextureManager textureManager{ logger };
	ShaderManager shaderManager{ logger };

	PhysicsSystem physicsSystem(logger, setting, componentManager);
	SpriteSystem spriteSystem(logger, setting, componentManager);



	auto&& physicsStorage = componentManager.get<core::ecs::PhysicsComponent>();
	auto&& spriteStorage = componentManager.get<core::ecs::SpriteComponent>();

	//graphics

	{
		auto temp_window = graphics::gl::initGlad(Window::GLInfo(), logger);
	}

	Window window{ WindowInfo{ 1920, 1080, "Neon Engine" }, logger };
	window.makeContextCurrent();

	FrameRateController frameRateController;

	frameRateController.setFrameRate(frameRate);
	Renderer renderer;


	/*gameplay*/
	//graphics
	Program program;
	try {
		textureManager.loadTexture(picPath);

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



	std::vector<Vertex> vertices = {
	{ {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },  // 左下
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


	auto texture = textureManager.loadTexture(picPath).lock();
	if (!texture) {
		logger.error("Failed to load texture.");
		return EXIT_FAILURE;
	}

	SpriteRegion region{ 0, 0, texture->info().width, texture->info().height };

	for (size_t i = 0; i < 1; i++)
	{
		auto&& entity = entityManager.createEntity();


		physicsStorage.addTo(
			entity,
			core::ecs::PhysicsComponent
			{
				.position = glm::vec3{0, 0, 0},
				.velocity = randomUnitVector() * 0.5f
			}
		);

		spriteStorage.addTo
		(
			entity,
			core::ecs::SpriteComponent{ mesh, *texture, region }
		);
	}



	float averageFrameRate = 0.0f;
	float frameRateStdDev = 0.0f;
	float frameRateStability = 0.0f;

	int frameCount = 0;

	constexpr int sampleCount = 60;
	std::deque<float> frameRates;
	glfwSwapInterval(0);
	/* game loop */
	while (!glfwWindowShouldClose(window.getGLFWwindow()))
	{
		window.pollEvents();

		// ecs
		physicsSystem.update();

		// graphics
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto&& spriteComponents = componentManager.get<core::ecs::SpriteComponent>();
		auto&& physicComponents = componentManager.get<core::ecs::PhysicsComponent>();

		for (auto&& [entity, spriteComponent] : spriteComponents)
		{
			auto&& physicsComponent = physicComponents.get(entity);
			renderer.Draw(
				spriteComponent,
				physicsComponent,
				program,
				glm::mat4(1.0f),
				glm::mat4(1.0f)
			);
		}

		window.swapBuffers();

		frameRateController.checkAndWait();

		const float currentFrameRate =
			frameRateController.getActualFrameRate();

		// --------------------------------------------------
		// 最近 N 帧 FPS
		// --------------------------------------------------

		frameRates.push_back(currentFrameRate);

		if (frameRates.size() > sampleCount)
			frameRates.pop_front();

		// --------------------------------------------------
		// 平均 FPS
		// --------------------------------------------------

		float sum = 0.0f;

		for (float fps : frameRates)
			sum += fps;

		averageFrameRate = sum / frameRates.size();

		// --------------------------------------------------
		// FPS 标准差
		// --------------------------------------------------

		float variance = 0.0f;

		for (float fps : frameRates)
		{
			const float difference = fps - averageFrameRate;
			variance += difference * difference;
		}

		variance /= frameRates.size();

		frameRateStdDev = std::sqrt(variance);

		// --------------------------------------------------
		// 稳定度（变异系数）
		//
		// 0%   = 完全稳定
		// 1%   = 非常稳定
		// 5%   = 有一定波动
		// 10%+ = 波动明显
		// --------------------------------------------------

		if (averageFrameRate > 0.0f)
		{
			frameRateStability =
				frameRateStdDev / averageFrameRate * 100.0f;
		}

		frameCount++;
	}

	logger.info("Average Frame Rate: {:.2f} FPS", averageFrameRate);
	logger.info(
		"FrameRate difference from target: {:.2f} %",
		(averageFrameRate - frameRate) / frameRate * 100.0f
	);

	logger.info(
		"FrameRate stability (CV): {:.2f} %",
		frameRateStability
	);
}