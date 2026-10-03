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

#include "core/type_system/type_id.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <random>
#include <ranges>

#include <cmath>
#include <deque>

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

// ---------- 随机单位向量（二维单位圆盘内均匀分布） ----------
inline glm::vec3 randomUnitVector(std::mt19937& randomEngine) {
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
	glm::vec3 v;
	do {
		v = glm::vec3(dist(randomEngine), dist(randomEngine), 0);
	} while (glm::dot(v, v) > 1.0f || glm::dot(v, v) < 1e-6f); // 拒绝采样，保证在单位圆盘内且非零
	return glm::normalize(v);
}

int main(int argc, char* argv[]) {

	int frameRate = 1000; // 设置基准测试的目标帧率
	using namespace neon;
	using
		neon::core::Logger, neon::core::Setting,
		neon::core::ecs::EntityManager, neon::core::ecs::ComponentManager,
		neon::core::ecs::PhysicsSystem, neon::graphics::FrameRateController,
		neon::graphics::gl::Window, neon::graphics::Renderer,
		neon::core::ecs::SpriteSystem, neon::graphics::gl::Mesh,
		neon::graphics::gl::Texture,
		neon::graphics::gl::Vertex, neon::core::ecs::SpriteRegion,
		neon::graphics::gl::ShaderManager, neon::graphics::gl::Program,
		neon::graphics::gl::Shader, neon::graphics::WindowInfo,
		neon::core::ecs::Entity, neon::graphics::gl::UniformBuffer;

	std::system("chcp 65001 > nul");  // 65001 就是 UTF-8

	/*申明*/
	//common
	Logger logger{};
	logger.setLogLevel(Logger::LogLevel::debug);
	Setting setting;
	logger.info("Game path: {}", argv[0]);
	/*创建*/
	//ecs
	EntityManager entityManager{ logger };
	ComponentManager componentManager;
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
	logger.info("Main: OpenGL window and context are ready.");
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
	frameRateController.setFrameRate(frameRate);
	Renderer renderer;


	/*gameplay*/
	//graphics
	Program program;
	try {
		logger.info("Main: loading and linking shaders.");
		shaderManager.init(); // Initialize the shader manager
		shaderManager.loadShaders({// Load and compile shaders from specified file paths and types
			{"D:/neon/program/project/cpp/NeonEngine/Engine/assets/shaders/vertex_shader.glsl", GL_VERTEX_SHADER},
			{ "D:/neon/program/project/cpp/NeonEngine/Engine/assets/shaders/fragment_shader.glsl", GL_FRAGMENT_SHADER }
			});
		shaderManager.linkPrograms({ "D:/neon/program/project/cpp/NeonEngine/Engine/assets/shaders/vertex_shader.glsl", "D:/neon/program/project/cpp/NeonEngine/Engine/assets/shaders/fragment_shader.glsl" });


		program = shaderManager.buildShader();
		logger.info("Main: shader program is ready.");
	}
	catch (const std::runtime_error& e)
	{
		logger.error("Main: shader initialization failed: {}", e.what());
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
	logger.info("Main: sprite mesh is ready.");

	/*creat entities */
	constexpr std::size_t entityCount = 10000 * 10;
	SpriteRegion region{ 0, 0, 1, 1 };
	std::deque<Texture> entityTextures;
	std::mt19937 randomEngine{ std::random_device{}() };
	constexpr std::array<std::array<std::uint8_t, 4>, 4> textureColors{ {
		{{255, 80, 80, 255}},
		{{80, 255, 120, 255}},
		{{80, 140, 255, 255}},
		{{255, 220, 80, 255}}
	} };
	for (const auto& color : textureColors)
	{
		entityTextures.emplace_back(Texture::Info{});
		auto& texture = entityTextures.back();
		texture.allocateStorage(1, GL_RGBA8, 1, 1);
		texture.uploadData(0, GL_RGBA, GL_UNSIGNED_BYTE, color.data());
		texture.setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		texture.setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	std::uniform_int_distribution<std::size_t> textureIndex(0, entityTextures.size() - 1);
	logger.info("Main: creating {} entities and {} shared 1x1 color textures.", entityCount, entityTextures.size());

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
	for (std::size_t i = 0; i < entityCount; ++i)
	{
		auto&& entity = entityManager.createEntity();
		const glm::vec3 initialVelocity = randomUnitVector(randomEngine);

		physicsStorage.addTo(
			entity,
			core::ecs::PhysicsComponent
			{
				.position = glm::vec3{0.5, 0.5, 0},
				.velocity = initialVelocity
			}
		);

		spriteStorage.addTo
		(
			entity,
			core::ecs::SpriteComponent{ mesh, entityTextures[textureIndex(randomEngine)], region, setting }
		);

		if ((i + 1) % 1000 == 0) {
			logger.info("Main: initialized {}/{} entities.", i + 1, entityCount);
		}
	}
	logger.info("Main: entity and texture creation completed.");





	float averageFrameRate = 0.0f;
	float frameRateStdDev = 0.0f;
	float frameRateStability = 0.0f;

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
	enum class SpriteRenderMode
	{
		Individual,
		Array,
		TextureGrouped
	};
	SpriteRenderMode renderMode = SpriteRenderMode::Array;
	constexpr std::array<int, 3> renderModeKeys{ GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3 };
	constexpr std::array<const char*, 3> renderModeNames{
		"Individual Sprite Renderer",
		"Sprite Array Renderer",
		"Texture Grouped Sprite Renderer"
	};
	std::array<bool, renderModeKeys.size()> renderModeKeyWasPressed{};
	auto modeIntervalStartedAt = std::chrono::steady_clock::now();
	std::size_t modeIntervalFrameCount = 0;
	int frameCount = 0;
	constexpr int diagnosticIntervalFrames = 180;
	using DiagnosticClock = std::chrono::steady_clock;
	logger.info("Main: entering render loop; diagnostic checkpoint every {} frames.", diagnosticIntervalFrames);
	/* game loop */
	while (!glfwWindowShouldClose(window.getGLFWwindow()))
	{
		const bool diagnosticFrame = frameCount % diagnosticIntervalFrames == 0;
		const auto frameStart = diagnosticFrame ? DiagnosticClock::now() : DiagnosticClock::time_point{};
		DiagnosticClock::time_point stageStart{};
		if (diagnosticFrame) {
			logger.debug("Frame {}: polling window events.", frameCount);
			stageStart = DiagnosticClock::now();
		}

		window.pollEvents();
		if (diagnosticFrame) {
			const double stageMs = std::chrono::duration<double, std::milli>(DiagnosticClock::now() - stageStart).count();
			logger.debug("Frame {}: window events complete in {:.2f} ms; checking input.", frameCount, stageMs);
		}

		for (std::size_t i = 0; i < renderModeKeys.size(); ++i)
		{
			const bool keyIsPressed = glfwGetKey(window.getGLFWwindow(), renderModeKeys[i]) == GLFW_PRESS;
			if (keyIsPressed && !renderModeKeyWasPressed[i])
			{
				const auto nextMode = static_cast<SpriteRenderMode>(i);
				if (nextMode != renderMode)
				{
					const auto now = std::chrono::steady_clock::now();
					const double elapsedSeconds =
						std::chrono::duration<double>(now - modeIntervalStartedAt).count();
					if (modeIntervalFrameCount > 0 && elapsedSeconds > 0.0)
					{
						const double averageFrameLatencyMs =
							elapsedSeconds * 1000.0 / modeIntervalFrameCount;
						const double averageFrameRateForMode =
							modeIntervalFrameCount / elapsedSeconds;
						logger.info(
							"Renderer mode {} finished: average frame latency {:.3f} ms, average frame rate {:.2f} FPS ({} frames)",
							renderModeNames[static_cast<std::size_t>(renderMode)],
							averageFrameLatencyMs,
							averageFrameRateForMode,
							modeIntervalFrameCount
						);
					}

					renderMode = nextMode;
					modeIntervalStartedAt = now;
					modeIntervalFrameCount = 0;
					logger.info("Renderer mode: {}", renderModeNames[i]);
				}
			}
			renderModeKeyWasPressed[i] = keyIsPressed;
		}

		// ecs
		if (diagnosticFrame) {
			logger.debug("Frame {}: physics update started.", frameCount);
			stageStart = DiagnosticClock::now();
		}
		physicsSystem.update();
		if (diagnosticFrame) {
			const double stageMs = std::chrono::duration<double, std::milli>(DiagnosticClock::now() - stageStart).count();
			logger.debug("Frame {}: physics update complete in {:.2f} ms; collecting render components.", frameCount, stageMs);
		}

		//for (auto& component : physicsStorage) {
		//	std::cout << component << std::endl;
		//}

		//graphics
		// 清屏
		if (diagnosticFrame) {
			logger.debug("Frame {}: clearing frame and gathering render components.", frameCount);
			stageStart = DiagnosticClock::now();
		}
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto& spriteComponents =
			componentManager.get<core::ecs::SpriteComponent>();

		auto& physicsComponents =
			componentManager.get<core::ecs::PhysicsComponent>();

		const auto& spriteEntities = spriteComponents.getEntities();

		auto& sprites = spriteComponents.getAllComponents();
		auto physics = physicsComponents.getForEntities(spriteEntities);
		if (diagnosticFrame) {
			const double stageMs = std::chrono::duration<double, std::milli>(DiagnosticClock::now() - stageStart).count();
			logger.debug("Frame {}: cleared frame and prepared {} render components in {:.2f} ms.", frameCount, sprites.size(), stageMs);
			stageStart = DiagnosticClock::now();
		}

		switch (renderMode)
		{
		case SpriteRenderMode::Individual:
			for (std::size_t i = 0; i < sprites.size() && i < physics.size(); ++i)
			{
				renderer.Draw(sprites[i], physics[i], program, view, projection);
			}
			break;
		case SpriteRenderMode::Array:
			renderer.DrawSpriteArray(
				sprites,
				physics,
				program,
				shareUBO,
				view,
				projection
			);
			break;
		case SpriteRenderMode::TextureGrouped:
			renderer.DrawSpriteArrayByTexture(
				sprites,
				physics,
				program,
				shareUBO,
				view,
				projection
			);
			break;
		}


		if (diagnosticFrame) {
			const double stageMs = std::chrono::duration<double, std::milli>(DiagnosticClock::now() - stageStart).count();
			logger.debug("Frame {}: rendering complete in {:.2f} ms; swapping buffers.", frameCount, stageMs);
			stageStart = DiagnosticClock::now();
		}
		window.swapBuffers();
		if (diagnosticFrame) {
			const double stageMs = std::chrono::duration<double, std::milli>(DiagnosticClock::now() - stageStart).count();
			logger.debug("Frame {}: buffer swap complete in {:.2f} ms; frame limiter started.", frameCount, stageMs);
			stageStart = DiagnosticClock::now();
		}

		frameRateController.checkAndWait();
		++modeIntervalFrameCount;

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
		if (diagnosticFrame) {
			const double frameMs = std::chrono::duration<double, std::milli>(DiagnosticClock::now() - frameStart).count();
			logger.info(
				"Main loop heartbeat: frame={}, frame time={:.2f} ms, FPS={:.1f}, average={:.1f}, stability={:.2f}%.",
				frameCount,
				frameMs,
				currentFrameRate,
				averageFrameRate,
				frameRateStability
			);
		}
	}
	logger.info("Main: render loop exited at frame {}.", frameCount);

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
