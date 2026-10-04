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
#include <optional>
#include <random>
#include <ranges>
#include <string>

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

// ---------- 随机初始位置（可见区内切圆内均匀取点，避免铺满整个方形屏幕） ----------
inline glm::vec3 randomSpawnPosition(std::mt19937& randomEngine) {
	constexpr float spawnRadius = 1.0f; // 可见区 [-1,1]² 的内切圆半径
	std::uniform_real_distribution<float> unit(0.0f, 1.0f);
	// 半径取 sqrt 才能保证圆内按面积均匀，否则点会向圆心堆积
	const float radius = spawnRadius * std::sqrt(unit(randomEngine));
	const glm::vec3 direction = randomUnitVector(randomEngine);
	return glm::vec3(direction.x * radius, direction.y * radius, 0);
}

// =====================================================================
// 本次改动：只做拆分，不考虑复用。
// 把原先全部堆在 main 里的代码按“流程”拆成若干函数，
// 语句与执行顺序保持与原 main 一致，只是把局部变量集中到 AppContext。
// =====================================================================
namespace
{
	using namespace neon;

	/* ---------- 各流程共享的常量 ---------- */
	constexpr int sampleCount = 60;						// 帧率统计的采样窗口
	constexpr int diagnosticIntervalFrames = 180;		// 每隔多少帧打印一次诊断日志
	constexpr std::size_t entityCount = 1000;			// 创建的实体数量（两两引力是 O(N²)：1000 个约 50 万对/帧）
	constexpr int physicsLogInterval = 1000;			// 只跑物理模式下，每隔多少步打印一次能量
	constexpr float physicsOnlyFrameDelta = 1.0f / 60.0f; // 只跑物理模式下固定使用的帧时长，保证结果可复现

	using DiagnosticClock = std::chrono::steady_clock;

	// 三种精灵渲染模式
	enum class SpriteRenderMode
	{
		Individual,
		Array,
		TextureGrouped
	};

	constexpr std::array<int, 3> renderModeKeys{ GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3 };
	constexpr std::array<const char*, 3> renderModeNames{
		"Individual Sprite Renderer",
		"Sprite Array Renderer",
		"Texture Grouped Sprite Renderer"
	};

	/* ---------- 流程 0：运行状态 ----------
	 * 原先散落在 main 里的局部变量集中放在这里，供各流程函数直接使用。
	 * 成员声明顺序与原 main 中“申明/创建”的顺序基本一致。
	 */
	struct AppContext
	{
		// 日志与配置
		core::Logger logger{};
		core::Setting setting;

		// ecs
		core::ecs::EntityManager entityManager{ logger };
		core::ecs::ComponentManager componentManager;
		core::ecs::PhysicsSystem physicsSystem{ logger, setting, componentManager };
		core::ecs::SpriteSystem spriteSystem{ logger, setting, componentManager };

		// graphics
		graphics::gl::ShaderManager shaderManager{ logger };
		std::optional<graphics::gl::Window> window;		// glad/GLFW 就绪后再构造
		graphics::FrameRateController frameRateController;
		graphics::Renderer renderer;
		graphics::gl::Program program;

		// gameplay（网格与纹理都在场景流程里创建）
		std::optional<graphics::gl::Mesh> mesh;
		std::deque<graphics::gl::Texture> entityTextures;
		std::mt19937 randomEngine{ std::random_device{}() };

		// 渲染状态
		graphics::gl::UniformBuffer shareUBO;
		glm::mat4 view{ 1.0f };
		glm::mat4 projection{ 1.0f };
		SpriteRenderMode renderMode{ SpriteRenderMode::Array };
		std::array<bool, renderModeKeys.size()> renderModeKeyWasPressed{};
		DiagnosticClock::time_point modeIntervalStartedAt{};
		std::size_t modeIntervalFrameCount = 0;
		int frameCount = 0;

		// 帧率统计
		std::deque<float> frameRates;
		float averageFrameRate = 0.0f;
		float frameRateStdDev = 0.0f;
		float frameRateStability = 0.0f;

		// 快速验证模式：只跑物理，不渲染、不限帧，跑满 physicsStepBudget 步后退出
		bool physicsOnly = false;
		int physicsStepBudget = 20000;
	};

	/* ---------- 流程 1：启动（日志与设置） ---------- */
	void startup(AppContext& app, char* argv[])
	{
		app.logger.setLogLevel(core::Logger::LogLevel::info);
		app.logger.info("Game path: {}", argv[0]);
	}

	/* ---------- 流程 1b：确认物理边界 ----------
	 * 边界种类属于启动配置：在这里（命令行解析之后、主循环之前）确认一次，
	 * 运行期间不再改变——物理系统没有切换边界的接口。
	 */
	void initPhysicsBoundary(AppContext& app)
	{
		app.physicsSystem.initializeBoundary();
	}

	/* ---------- 流程 2：创建窗口与 OpenGL 上下文 ---------- */
	void initWindow(AppContext& app)
	{
		{
			auto temp_window = graphics::gl::initGlad(graphics::gl::Window::GLInfo(), app.logger);
		}

		app.window.emplace(
			graphics::WindowInfo{
				app.setting.windowInfo.width,
				app.setting.windowInfo.height,
				app.setting.windowInfo.title
			},
			app.logger
		);

		auto& window = *app.window;
		window.makeContextCurrent();
		app.logger.info("Main: OpenGL window and context are ready.");
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

		app.frameRateController.setFrameRate(app.setting.FrameRate);
	}

	/* ---------- 流程 3：加载着色器并生成着色器程序 ---------- */
	bool initGraphics(AppContext& app)
	{
		try {
			app.logger.info("Main: loading and linking shaders.");
			app.shaderManager.init(); // Initialize the shader manager
			const std::string vertexShaderPath = "assets/shaders/vertex_shader.glsl";
			const std::string fragmentShaderPath = "assets/shaders/fragment_shader.glsl";
			app.shaderManager.loadShaders({// Load and compile shaders from specified file paths and types
				{vertexShaderPath, GL_VERTEX_SHADER},
				{fragmentShaderPath, GL_FRAGMENT_SHADER}
				});
			app.shaderManager.linkPrograms({ vertexShaderPath, fragmentShaderPath });


			app.program = app.shaderManager.buildShader();
			app.logger.info("Main: shader program is ready.");
		}
		catch (const std::runtime_error& e)
		{
			app.logger.error("Main: shader initialization failed: {}", e.what());
			return false;
		}

		return true;
	}

	/* ---------- 流程 4：构建场景（网格、纹理与实体） ---------- */
	void initScene(AppContext& app)
	{
		auto& physicsStorage = app.componentManager.get<core::ecs::PhysicsComponent>();
		auto& spriteStorage = app.componentManager.get<core::ecs::SpriteComponent>();

		//ecs：第一个实体只挂物理组件，不参与渲染
		auto&& entity = app.entityManager.createEntity();

		physicsStorage.addTo(
			entity,
			core::ecs::PhysicsComponent{
				.position = randomSpawnPosition(app.randomEngine),
				.velocity = glm::vec3{0.5, 0.2, 0}
			}
		);

		std::vector<graphics::gl::Vertex> vertices = {
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
		app.mesh.emplace(vertices, indices);
		app.logger.info("Main: sprite mesh is ready.");

		/*creat entities */
		core::ecs::SpriteRegion region{ 0, 0, 1, 1 };
		constexpr std::array<std::array<std::uint8_t, 4>, 2> textureColors{ {
			{{0, 80, 255, 0}}, // 所有普通粒子的颜色
			{{255, 80, 80, 255}}   // 唯一例外粒子的颜色
		} };
		for (const auto& color : textureColors)
		{
			app.entityTextures.emplace_back(graphics::gl::Texture::Info{});
			auto& texture = app.entityTextures.back();
			texture.allocateStorage(1, GL_RGBA8, 1, 1);
			texture.uploadData(0, GL_RGBA, GL_UNSIGNED_BYTE, color.data());
			texture.setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			texture.setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		app.logger.info("Main: creating {} entities and {} shared 1x1 color textures.", entityCount, app.entityTextures.size());

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			auto&& entity = app.entityManager.createEntity();
			const glm::vec3 initialPosition = randomSpawnPosition(app.randomEngine);

			physicsStorage.addTo(
				entity,
				core::ecs::PhysicsComponent
				{
					.position = initialPosition,
					.velocity = glm::vec3{0, 0, 0} // 初速度为 0：先静止，再由两两引力拉成一团，观感更直观
				}
			);

			spriteStorage.addTo
			(
				entity,
				core::ecs::SpriteComponent{ *app.mesh, app.entityTextures[i == 0 ? 1 : 0], region, app.setting }
			);

			if ((i + 1) % 10000 == 0) {
				app.logger.info("Main: initialized {}/{} entities.", i + 1, entityCount);
			}
		}
		app.logger.info("Main: entity and texture creation completed.");
	}

	/* ---------- 流程 5：准备渲染状态（共享 UBO、相机矩阵、渲染模式） ---------- */
	void initRenderState(AppContext& app)
	{
		glfwSwapInterval(0);

		app.shareUBO.allocate(sizeof(glm::mat4) * 2);

		app.view = glm::mat4{ 1.0f };
		app.projection = glm::mat4{ 1.0f };
		app.shareUBO.update(0, sizeof(glm::mat4), glm::value_ptr(app.view));
		app.shareUBO.update(sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(app.projection));
		app.shareUBO.bindBase(0);

		app.renderMode = SpriteRenderMode::TextureGrouped; // 默认用第三种渲染模式
		app.renderModeKeyWasPressed.fill(false);
		app.modeIntervalStartedAt = DiagnosticClock::now();
		app.modeIntervalFrameCount = 0;
		app.frameCount = 0;
		app.logger.info(
			"Main: entering render loop with {}; diagnostic checkpoint every {} frames.",
			renderModeNames[static_cast<std::size_t>(app.renderMode)],
			diagnosticIntervalFrames
		);
	}

	/* ---------- 流程 6：每帧输入（轮询事件 + 切换渲染模式） ---------- */
	void updateFrameInput(AppContext& app, bool diagnosticFrame)
	{
		auto& window = *app.window;
		DiagnosticClock::time_point stageStart{};
		if (diagnosticFrame) {
			app.logger.debug("Frame {}: polling window events.", app.frameCount);
			stageStart = DiagnosticClock::now();
		}

		window.pollEvents();
		if (diagnosticFrame) {
			const double stageMs = std::chrono::duration<double, std::milli>(DiagnosticClock::now() - stageStart).count();
			app.logger.debug("Frame {}: window events complete in {:.2f} ms; checking input.", app.frameCount, stageMs);
		}

		for (std::size_t i = 0; i < renderModeKeys.size(); ++i)
		{
			const bool keyIsPressed = glfwGetKey(window.getGLFWwindow(), renderModeKeys[i]) == GLFW_PRESS;
			if (keyIsPressed && !app.renderModeKeyWasPressed[i])
			{
				const auto nextMode = static_cast<SpriteRenderMode>(i);
				if (nextMode != app.renderMode)
				{
					const auto now = std::chrono::steady_clock::now();
					const double elapsedSeconds =
						std::chrono::duration<double>(now - app.modeIntervalStartedAt).count();
					if (app.modeIntervalFrameCount > 0 && elapsedSeconds > 0.0)
					{
						const double averageFrameLatencyMs =
							elapsedSeconds * 1000.0 / app.modeIntervalFrameCount;
						const double averageFrameRateForMode =
							app.modeIntervalFrameCount / elapsedSeconds;
						app.logger.info(
							"Renderer mode {} finished: average frame latency {:.3f} ms, average frame rate {:.2f} FPS ({} frames)",
							renderModeNames[static_cast<std::size_t>(app.renderMode)],
							averageFrameLatencyMs,
							averageFrameRateForMode,
							app.modeIntervalFrameCount
						);
					}

					app.renderMode = nextMode;
					app.modeIntervalStartedAt = now;
					app.modeIntervalFrameCount = 0;
					app.logger.info("Renderer mode: {}", renderModeNames[i]);
				}
			}
			app.renderModeKeyWasPressed[i] = keyIsPressed;
		}
	}

	/* ---------- 流程 7：每帧更新（物理系统） ---------- */
	void updateFrameSimulation(AppContext& app, bool diagnosticFrame, float frameDeltaTime)
	{
		DiagnosticClock::time_point stageStart{};
		if (diagnosticFrame) {
			app.logger.debug("Frame {}: physics update started.", app.frameCount);
			stageStart = DiagnosticClock::now();
		}

		app.physicsSystem.update(frameDeltaTime);

		if (diagnosticFrame) {
			const double stageMs = std::chrono::duration<double, std::milli>(DiagnosticClock::now() - stageStart).count();
			app.logger.debug("Frame {}: physics update complete in {:.2f} ms; collecting render components.", app.frameCount, stageMs);
		}
	}

	/* ---------- 流程 8：每帧渲染（清屏、收集组件、提交绘制） ---------- */
	void renderFrame(AppContext& app, bool diagnosticFrame)
	{
		DiagnosticClock::time_point stageStart{};
		if (diagnosticFrame) {
			app.logger.debug("Frame {}: clearing frame and gathering render components.", app.frameCount);
			stageStart = DiagnosticClock::now();
		}

		// 清屏
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto& spriteComponents =
			app.componentManager.get<core::ecs::SpriteComponent>();

		auto& physicsComponents =
			app.componentManager.get<core::ecs::PhysicsComponent>();

		const auto& spriteEntities = spriteComponents.getEntities();

		auto& sprites = spriteComponents.getAllComponents();
		auto physics = physicsComponents.getForEntities(spriteEntities);
		if (diagnosticFrame) {
			const double stageMs = std::chrono::duration<double, std::milli>(DiagnosticClock::now() - stageStart).count();
			app.logger.debug("Frame {}: cleared frame and prepared {} render components in {:.2f} ms.", app.frameCount, sprites.size(), stageMs);
			stageStart = DiagnosticClock::now();
		}

		switch (app.renderMode)
		{
		case SpriteRenderMode::Individual:
			for (std::size_t i = 0; i < sprites.size() && i < physics.size(); ++i)
			{
				app.renderer.Draw(sprites[i], physics[i], app.program, app.view, app.projection);
			}
			break;
		case SpriteRenderMode::Array:
			app.renderer.DrawSpriteArray(
				sprites,
				physics,
				app.program,
				app.shareUBO,
				app.view,
				app.projection
			);
			break;
		case SpriteRenderMode::TextureGrouped:
			app.renderer.DrawSpriteArrayByTexture(
				sprites,
				physics,
				app.program,
				app.shareUBO,
				app.view,
				app.projection
			);
			break;
		}

		if (diagnosticFrame) {
			const double stageMs = std::chrono::duration<double, std::milli>(DiagnosticClock::now() - stageStart).count();
			app.logger.debug("Frame {}: rendering complete in {:.2f} ms; swapping buffers.", app.frameCount, stageMs);
		}
	}

	/* ---------- 流程 9：帧率统计（平均值、标准差、稳定度） ---------- */
	void updateFrameRateStatistics(AppContext& app, float currentFrameRate)
	{
		// --------------------------------------------------
		// 最近 N 帧 FPS
		// --------------------------------------------------

		app.frameRates.push_back(currentFrameRate);

		if (app.frameRates.size() > sampleCount)
			app.frameRates.pop_front();

		// --------------------------------------------------
		// 平均 FPS
		// --------------------------------------------------

		float sum = 0.0f;

		for (float fps : app.frameRates)
			sum += fps;

		app.averageFrameRate = sum / app.frameRates.size();

		// --------------------------------------------------
		// FPS 标准差
		// --------------------------------------------------

		float variance = 0.0f;

		for (float fps : app.frameRates)
		{
			const float difference = fps - app.averageFrameRate;
			variance += difference * difference;
		}

		variance /= app.frameRates.size();

		app.frameRateStdDev = std::sqrt(variance);

		// --------------------------------------------------
		// 稳定度（变异系数）
		//
		// 0%   = 完全稳定
		// 1%   = 非常稳定
		// 5%   = 有一定波动
		// 10%+ = 波动明显
		// --------------------------------------------------

		if (app.averageFrameRate > 0.0f)
		{
			app.frameRateStability =
				app.frameRateStdDev / app.averageFrameRate * 100.0f;
		}
	}

	/* ---------- 流程 10：每帧收尾（交换缓冲、帧率限制、统计与心跳日志） ---------- */
	void presentFrame(AppContext& app, bool diagnosticFrame, DiagnosticClock::time_point frameStart)
	{
		DiagnosticClock::time_point stageStart{};
		if (diagnosticFrame) {
			stageStart = DiagnosticClock::now();
		}

		app.window->swapBuffers();
		if (diagnosticFrame) {
			const double stageMs = std::chrono::duration<double, std::milli>(DiagnosticClock::now() - stageStart).count();
			app.logger.debug("Frame {}: buffer swap complete in {:.2f} ms; frame limiter started.", app.frameCount, stageMs);
		}

		app.frameRateController.checkAndWait();
		++app.modeIntervalFrameCount;

		const float currentFrameRate =
			app.frameRateController.getActualFrameRate();

		updateFrameRateStatistics(app, currentFrameRate);

		++app.frameCount;
		if (diagnosticFrame) {
			const double frameMs = std::chrono::duration<double, std::milli>(DiagnosticClock::now() - frameStart).count();
			const float kineticEnergy = app.physicsSystem.getTotalKineticEnergy();
			const float potentialEnergy = app.physicsSystem.getTotalPotentialEnergy();
			app.logger.info(
				"Main loop heartbeat: frame={}, sim time={:.2f} s, substeps={}, boundary hits={}, frame time={:.2f} ms, FPS={:.1f}, average={:.1f}, stability={:.2f}%, kinetic={:.4f}, potential={:.4f}, total energy={:.4f}.",
				app.frameCount,
				app.physicsSystem.getSimulatedTime(),
				app.physicsSystem.getLastSubsteps(),
				app.physicsSystem.getTotalBoundaryHits(),
				frameMs,
				currentFrameRate,
				app.averageFrameRate,
				app.frameRateStability,
				kineticEnergy,
				potentialEnergy,
				kineticEnergy + potentialEnergy
			);
		}
	}

	/* ---------- 流程 10b：打印能量（快速验证用） ---------- */
	void logEnergy(AppContext& app)
	{
		const float kineticEnergy = app.physicsSystem.getTotalKineticEnergy();
		const float potentialEnergy = app.physicsSystem.getTotalPotentialEnergy();
		const core::ecs::RepulsionStats& repulsion = app.physicsSystem.getLastRepulsionStats();
		app.logger.info(
			"Energy: step={}, substeps={}, simulated time={:.2f} s, boundary hits={}, kinetic={:.4f}, potential={:.4f}, total={:.4f}.",
			app.frameCount,
			app.physicsSystem.getLastSubsteps(),
			app.physicsSystem.getSimulatedTime(),
			app.physicsSystem.getTotalBoundaryHits(),
			kineticEnergy,
			potentialEnergy,
			kineticEnergy + potentialEnergy
		);
		app.logger.info(
			"Repulsion: active pairs={}, clamped pairs={}, sum |dv|={:.4f} per frame, potential={:.4f}.",
			repulsion.pairCount,
			repulsion.clampedPairCount,
			repulsion.velocityChange,
			repulsion.potentialEnergy
		);
	}

	/* ---------- 流程 11：主循环 ---------- */
	void runMainLoop(AppContext& app)
	{
		auto& window = *app.window;
		if (app.physicsOnly) {
			app.logger.info(
				"Main: physics-only mode (no rendering, no frame limiter), budget {} steps.",
				app.physicsStepBudget
			);
			logEnergy(app);
		}
		/* game loop */
		auto previousFrameTime = DiagnosticClock::now();
		while (!glfwWindowShouldClose(window.getGLFWwindow()))
		{
			const bool diagnosticFrame =
				!app.physicsOnly && app.frameCount % diagnosticIntervalFrames == 0;
			const auto frameStart = diagnosticFrame ? DiagnosticClock::now() : DiagnosticClock::time_point{};

			// 本帧的真实时长：只跑物理模式用固定值（结果可复现），渲染模式用实测值
			const auto now = DiagnosticClock::now();
			const float frameDeltaTime = app.physicsOnly
				? physicsOnlyFrameDelta
				: std::chrono::duration<float>(now - previousFrameTime).count();
			previousFrameTime = now;

			updateFrameInput(app, diagnosticFrame);
			updateFrameSimulation(app, diagnosticFrame, frameDeltaTime);

			if (app.physicsOnly) {
				// 快速验证：跳过渲染与帧率限制，只推进物理，并定期打印能量
				++app.frameCount;
				if (app.frameCount % physicsLogInterval == 0) logEnergy(app);
				if (app.frameCount >= app.physicsStepBudget) break;
				continue;
			}

			renderFrame(app, diagnosticFrame);
			presentFrame(app, diagnosticFrame, frameStart);
		}
		app.logger.info("Main: render loop exited at frame {}.", app.frameCount);
	}

	/* ---------- 流程 12：输出统计结果 ---------- */
	void reportSummary(AppContext& app)
	{
		app.logger.info("Average Frame Rate: {:.2f} FPS", app.averageFrameRate);
		app.logger.info(
			"FrameRate difference from target: {:.2f} %",
			(app.averageFrameRate - app.setting.FrameRate) / app.setting.FrameRate * 100.0f
		);

		app.logger.info(
			"FrameRate stability (CV): {:.2f} %",
			app.frameRateStability
		);

		app.logger.info("Total kinetic energy: {:.4f}", app.physicsSystem.getTotalKineticEnergy());
	}
}

int main(int argc, char* argv[])
{
	std::system("chcp 65001 > nul");  // 65001 就是 UTF-8

	AppContext app;

	// 快速验证用的命令行开关：
	//   --physics [步数]     只跑物理：不渲染、不限帧，跑满步数后退出（等价于 --steps）
	//   --steps <步数>       同上，显式指定步数
	//   --physics-rate <hz>  物理目标频率（内部子步步长 ≈ 1/hz），用于对比不同积分步长
	//   --framerate <n>      显示/限帧目标（只跑物理模式下没有意义）
	//   --no-repulsion       关闭近距斥力，用于对照实验
	//   --no-gravity         关闭引力，用于对照实验（粒子走直线，可直接观察周期边界）
	//   --boundary <名字>    边界处理方式（wrap 循环 / bounce 反弹）；默认 wrap，只在启动时生效
	for (int i = 1; i < argc; ++i) {
		const std::string argument = argv[i];
		if (argument == "--physics" || argument == "--steps") {
			app.physicsOnly = true;
			if (i + 1 < argc && argv[i + 1][0] != '-') {
				app.physicsStepBudget = std::atoi(argv[++i]);
			}
		}
		else if (argument == "--physics-rate" && i + 1 < argc) {
			app.setting.physicsRate = static_cast<float>(std::atof(argv[++i]));
		}
		else if (argument == "--framerate" && i + 1 < argc) {
			app.setting.FrameRate = std::atoi(argv[++i]);
		}
		else if (argument == "--no-repulsion") {
			app.setting.repulsionStrength = 0.0f;
		}
		else if (argument == "--no-gravity") {
			app.setting.gravity = 0.0f;
		}
		else if (argument == "--boundary" && i + 1 < argc) {
			const std::string name = argv[++i];
			if (!core::physics::parseBoundaryType(name, app.setting.boundary)) {
				app.logger.error(
					"--boundary {} is not a known boundary; using '{}'. Available: {}.",
					name,
					core::physics::boundaryTypeName(app.setting.boundary),
					core::physics::availableBoundaryNames()
				);
			}
		}
	}

	startup(app, argv);							// 流程 1：启动
	initPhysicsBoundary(app);					// 流程 1b：确认边界（启动时一次，之后不变）
	initWindow(app);							// 流程 2：创建窗口与上下文
	if (!app.physicsOnly && !initGraphics(app)) {					// 流程 3：着色器
		return EXIT_FAILURE;
	}
	initScene(app);								// 流程 4：网格、纹理与实体
	initRenderState(app);						// 流程 5：渲染状态
	runMainLoop(app);							// 流程 6~10：渲染主循环
	reportSummary(app);							// 流程 12：统计结果

	return EXIT_SUCCESS;
}
