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
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <ranges>

#include <cmath>
#include <deque>
#include <random>

#include <glm/gtc/type_ptr.hpp>

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

static std::mt19937& getRng() {
	static thread_local std::mt19937 rng{ std::random_device{}() };
	return rng;
}

// ---------- 1. 随机二维单位向量（圆周方向均匀） ----------
inline glm::vec3 randomUnitVector() {
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
	glm::vec3 v;
	do {
		v = glm::vec3(dist(getRng()), dist(getRng()), 0);
	} while (glm::dot(v, v) > 1.0f || glm::dot(v, v) < 1e-6f); // 拒绝采样，保证在单位球内且非零
	return glm::normalize(v);
}
int main(int argc, char* argv[]) {

	int frameRate = 60; // 设置目标帧率为 144 FPS
	std::string picPath = "1pic.png";
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
		neon::graphics::gl::Shader, neon::graphics::WindowInfo,
		neon::core::ecs::Entity, neon::graphics::gl::UniformBuffer;

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

	PhysicsSystem physicsSystem{ logger, setting, componentManager };
	SpriteSystem spriteSystem{ logger, setting, componentManager };

	auto&& entity = entityManager.createEntity();

	auto&& physicsStorage = componentManager.get<core::ecs::PhysicsComponent>();
	auto&& spriteStorage = componentManager.get<core::ecs::SpriteComponent>();

	//graphics

	{
		auto temp_window = graphics::gl::initGlad(Window::GLInfo(), logger);
	}

	Window window{ WindowInfo{ setting.windowInfo.width, setting.windowInfo.height, setting.windowInfo.title }, logger };
	window.makeContextCurrent();
	glfwSetFramebufferSizeCallback(
		window.getGLFWwindow(),
		[](GLFWwindow*, int width, int height)
		{
			glViewport(0, 0, width, height);
		}
	);
	int framebufferWidth = 0;
	int framebufferHeight = 0;
	glfwGetFramebufferSize(window.getGLFWwindow(), &framebufferWidth, &framebufferHeight);
	glViewport(0, 0, framebufferWidth, framebufferHeight);

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
	{ {-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },  // 左下
	{ { 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} },  // 右下
	{ {1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} },  // 右上
	{ {-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} }   // 左上
	};

	// 两个三角形，逆时针（CCW）为正面
	std::vector<unsigned int> indices = {
		0, 1, 2,   // 第一个三角形：左下 → 右下 → 右上
		0, 2, 3    // 第二个三角形：左下 → 右上 → 左上
	};
	Mesh mesh{ vertices, indices };


	auto texture_obj = textureManager.loadTexture(picPath).lock();
	if (!texture_obj) {
		logger.error("Failed to load texture.");
		return EXIT_FAILURE;
	}

	SpriteRegion region{ 0, 0, texture_obj->info().width, texture_obj->info().height };

	// 外层 entity 已经拥有 PhysicsComponent，因此也必须补充 SpriteComponent，
	// 否则后面的 spriteStorage.get(entity) 会访问不存在的组件。
	spriteStorage.addTo
	(
		entity,
		core::ecs::SpriteComponent{ mesh, *texture_obj, region, setting }
	);

	//background
	//auto&& entity = entityManager.createEntity();


	//physicsStorage.addTo(
	//	entity,
	//	core::ecs::PhysicsComponent
	//	{
	//		.position = glm::vec3{0, 0, 0},
	//		.velocity = glm::vec3{0, 0, 0}
	//	}
	//);

	//spriteStorage.addTo
	//(
	//	entity,
	//	core::ecs::SpriteComponent{ mesh, *textureManager.loadTexture("pixel_grid.png").lock(), region, setting }
	//);

	for (size_t i = 0; i < 10000; i++)
	{
		auto&& entity = entityManager.createEntity();


		physicsStorage.addTo(
			entity,
			core::ecs::PhysicsComponent
			{
				.position = glm::vec3{0.5, 0.5, 0},
				.velocity = randomUnitVector()
			}
		);

		spriteStorage.addTo
		(
			entity,
			core::ecs::SpriteComponent{ mesh, *texture_obj, region, setting }
		);
	}




	float averageFrameRate = 0.0f;
	float frameRateStdDev = 0.0f;
	float frameRateStability = 0.0f;

	auto&& spriteComponent = spriteStorage.get(entity);
	auto&& physicsComponent = physicsStorage.get(entity);

	constexpr int sampleCount = 60;
	std::deque<float> frameRates;
	glfwSwapInterval(0);

	UniformBuffer shareUBO;
	shareUBO.allocate(sizeof(glm::mat4) * 2);

	const glm::mat4 view{ 1.0f };
	const glm::mat4 projection{ 1.0f };
	shareUBO.update(0, sizeof(glm::mat4), glm::value_ptr(view));
	shareUBO.update(sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
	shareUBO.bindBase(0);
	bool useSpriteArrayRenderer = true;
	bool toggleKeyWasPressed = false;
	int frameCount = 0;
	/* game loop */
	while (!glfwWindowShouldClose(window.getGLFWwindow()))
	{

		window.pollEvents();

		const bool toggleKeyIsPressed =
			glfwGetKey(window.getGLFWwindow(), GLFW_KEY_TAB) == GLFW_PRESS;
		if (toggleKeyIsPressed && !toggleKeyWasPressed)
		{
			useSpriteArrayRenderer = !useSpriteArrayRenderer;
			logger.debug("Renderer mode toggled: {}", useSpriteArrayRenderer ? "Sprite Array Renderer" : "Individual Sprite Renderer");
		}
		toggleKeyWasPressed = toggleKeyIsPressed;

		// ecs
		physicsSystem.update();

		//for (auto& component : physicsStorage) {
		//	std::cout << component << std::endl;
		//}

		//graphics
		// 清屏
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto& spriteComponents =
			componentManager.get<core::ecs::SpriteComponent>();

		auto& physicsComponents =
			componentManager.get<core::ecs::PhysicsComponent>();

		const auto& spriteEntities = spriteComponents.getEntities();

		auto& sprites = spriteComponents.getAllComponents();
		auto physics = physicsComponents.getForEntities(spriteEntities);

		if (useSpriteArrayRenderer)
		{
			renderer.DrawSpriteArray(
				sprites,
				physics,
				program,
				shareUBO,
				view,
				projection
			);
		}
		else
		{
			for (std::size_t i = 0; i < sprites.size() && i < physics.size(); ++i)
			{
				renderer.Draw(sprites[i], physics[i], program, view, projection);
			}
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
