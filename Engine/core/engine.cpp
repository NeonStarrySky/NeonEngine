#include "engine.h"

#include <cmath>
#include <cstdint>
#include <glm/gtc/type_ptr.hpp>

namespace
{
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

}

namespace neon::core
{
	Engine::~Engine()
	{
		// GPU members use this context during their subsequent RAII destruction.
		context.makeCurrent();
	}

	bool Engine::init(const char* executablePath)
	{
		if (initialized) {
			return true;
		}
		startup(executablePath);
		initWindow();
		if (!initGraphics()) {
			return false;
		}
		initScene();
		initRenderState();
		initialized = true;
		return true;
	}

	void Engine::run()
	{
		if (!initialized) {
			throw std::logic_error("Engine must be initialized before run");
		}
		context.makeCurrent();
		runMainLoop();
		reportSummary();
	}

	int Engine::creatWindow(WindowInfo info)
	{
		return context.createWindow(info);
	}

	/* ---------- 流程 1：启动（日志与设置） ---------- */
	void Engine::startup(const char* executablePath)
	{
		logger.setLogLevel(core::Logger::LogLevel::debug);
		logger.info("Game path: {}", executablePath);
	}

	/* ---------- 流程 2：创建窗口与 OpenGL 上下文 ---------- */
	void Engine::initWindow()
	{
		creatWindow(
			graphics::WindowInfo{
				setting.windowInfo.width,
				setting.windowInfo.height,
				setting.windowInfo.title
			}
		);

		auto& window = getCurrentWindow();
		window.makeContextCurrent();
		glfwSetWindowUserPointer(window.getGLFWwindow(), this);
		logger.info("Engine: OpenGL window and context are ready.");
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

		frameRateController.setFrameRate(setting.FrameRate);
	}

	/* ---------- 流程 3：加载着色器并生成着色器程序 ---------- */
	bool Engine::initGraphics()
	{
		try {
			logger.info("Engine: loading and linking shaders.");
			shaderManager.init(); // Initialize the shader manager
			const std::string vertexShaderPath = "assets/shaders/vertex_shader.glsl";
			const std::string fragmentShaderPath = "assets/shaders/fragment_shader.glsl";
			shaderManager.loadShaders({// Load and compile shaders from specified file paths and types
				{vertexShaderPath, GL_VERTEX_SHADER},
				{fragmentShaderPath, GL_FRAGMENT_SHADER}
				});
			shaderManager.linkPrograms({ vertexShaderPath, fragmentShaderPath });


			program = shaderManager.buildShader();
			logger.info("Engine: shader program is ready.");
		}
		catch (const std::runtime_error& e)
		{
			logger.error("Engine: shader initialization failed: {}", e.what());
			return false;
		}

		return true;
	}

	/* ---------- 流程 4：构建场景（网格、纹理与实体） ---------- */
	void Engine::initScene()
	{
		auto& physicsStorage = componentManager.get<core::ecs::PhysicsComponent>();
		auto& spriteStorage = componentManager.get<core::ecs::SpriteComponent>();

		//ecs：第一个实体只挂物理组件，不参与渲染
		auto&& entity = entityManager.createEntity();

		physicsStorage.addTo(
			entity,
			core::ecs::PhysicsComponent{
				.position = randomSpawnPosition(randomEngine),
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
		mesh.emplace(vertices, indices);
		logger.info("Engine: sprite mesh is ready.");

		/*creat entities */
		core::ecs::SpriteRegion region{ 0, 0, 1, 1 };
		constexpr std::array<std::array<std::uint8_t, 4>, 4> textureColors{ {
			{{255, 80, 80, 255}},
			{{80, 255, 120, 255}},
			{{80, 140, 255, 255}},
			{{255, 220, 80, 255}}
		} };
		for (const auto& color : textureColors)
		{
			entityTextures.emplace_back(graphics::gl::Texture::Info{});
			auto& texture = entityTextures.back();
			texture.allocateStorage(1, GL_RGBA8, 1, 1);
			texture.uploadData(0, GL_RGBA, GL_UNSIGNED_BYTE, color.data());
			texture.setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			texture.setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		std::uniform_int_distribution<std::size_t> textureIndex(0, entityTextures.size() - 1);
		logger.info("Engine: creating {} entities and {} shared 1x1 color textures.", entityCount, entityTextures.size());

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			auto&& entity = entityManager.createEntity();
			const glm::vec3 initialPosition = randomSpawnPosition(randomEngine);
			const glm::vec3 initialVelocity = randomUnitVector(randomEngine);

			physicsStorage.addTo(
				entity,
				core::ecs::PhysicsComponent
				{
					.position = initialPosition,
					.velocity = initialVelocity
				}
			);

			spriteStorage.addTo
			(
				entity,
				core::ecs::SpriteComponent{ *mesh, entityTextures[textureIndex(randomEngine)], region, setting }
			);

			if ((i + 1) % 10000 == 0) {
				logger.info("Engine: initialized {}/{} entities.", i + 1, entityCount);
			}
		}
		logger.info("Engine: entity and texture creation completed.");
	}

	/* ---------- 流程 5：准备渲染状态（共享 UBO、相机矩阵、渲染模式） ---------- */
	void Engine::initRenderState()
	{
		glfwSwapInterval(0);

		shareUBO.allocate(sizeof(glm::mat4) * 2);

		view = glm::mat4{ 1.0f };
		projection = glm::mat4{ 1.0f };
		shareUBO.update(0, sizeof(glm::mat4), glm::value_ptr(view));
		shareUBO.update(sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
		shareUBO.bindBase(0);

		renderMode = SpriteRenderMode::Array;
		renderModeKeyWasPressed.fill(false);
		modeIntervalStartedAt = DiagnosticClock::now();
		modeIntervalFrameCount = 0;
		frameCount = 0;
		logger.info("Engine: entering render loop; diagnostic checkpoint every {} frames.", diagnosticIntervalFrames);
	}

	/* ---------- 流程 6：每帧输入（轮询事件 + 切换渲染模式） ---------- */
	void Engine::updateFrameInput(bool diagnosticFrame)
	{
		auto& window = getCurrentWindow();
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
	}

	/* ---------- 流程 7：每帧更新（物理系统） ---------- */
	void Engine::updateFrameSimulation(bool diagnosticFrame)
	{
		DiagnosticClock::time_point stageStart{};
		if (diagnosticFrame) {
			logger.debug("Frame {}: physics update started.", frameCount);
			stageStart = DiagnosticClock::now();
		}

		physicsSystem.update();

		if (diagnosticFrame) {
			const double stageMs = std::chrono::duration<double, std::milli>(DiagnosticClock::now() - stageStart).count();
			logger.debug("Frame {}: physics update complete in {:.2f} ms; collecting render components.", frameCount, stageMs);
		}
	}

	/* ---------- 流程 8：每帧渲染（清屏、收集组件、提交绘制） ---------- */
	void Engine::renderFrame(bool diagnosticFrame)
	{
		DiagnosticClock::time_point stageStart{};
		if (diagnosticFrame) {
			logger.debug("Frame {}: clearing frame and gathering render components.", frameCount);
			stageStart = DiagnosticClock::now();
		}

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
		}
	}

	/* ---------- 流程 9：帧率统计（平均值、标准差、稳定度） ---------- */
	void Engine::updateFrameRateStatistics(float currentFrameRate)
	{
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
	}

	/* ---------- 流程 10：每帧收尾（交换缓冲、帧率限制、统计与心跳日志） ---------- */
	void Engine::presentFrame(bool diagnosticFrame, DiagnosticClock::time_point frameStart)
	{
		DiagnosticClock::time_point stageStart{};
		if (diagnosticFrame) {
			stageStart = DiagnosticClock::now();
		}

		getCurrentWindow().swapBuffers();
		if (diagnosticFrame) {
			const double stageMs = std::chrono::duration<double, std::milli>(DiagnosticClock::now() - stageStart).count();
			logger.debug("Frame {}: buffer swap complete in {:.2f} ms; frame limiter started.", frameCount, stageMs);
		}

		frameRateController.checkAndWait();
		++modeIntervalFrameCount;

		const float currentFrameRate =
			frameRateController.getActualFrameRate();

		updateFrameRateStatistics(currentFrameRate);

		++frameCount;
		if (diagnosticFrame) {
			const double frameMs = std::chrono::duration<double, std::milli>(DiagnosticClock::now() - frameStart).count();
			logger.info(
				"Engine loop heartbeat: frame={}, frame time={:.2f} ms, FPS={:.1f}, average={:.1f}, stability={:.2f}%.",
				frameCount,
				frameMs,
				currentFrameRate,
				averageFrameRate,
				frameRateStability
			);
		}
	}

	/* ---------- 流程 11：主循环 ---------- */
	void Engine::runMainLoop()
	{
		auto& window = getCurrentWindow();
		/* game loop */
		while (!glfwWindowShouldClose(window.getGLFWwindow()))
		{
			const bool diagnosticFrame = frameCount % diagnosticIntervalFrames == 0;
			const auto frameStart = diagnosticFrame ? DiagnosticClock::now() : DiagnosticClock::time_point{};

			updateFrameInput(diagnosticFrame);
			updateFrameSimulation(diagnosticFrame);
			renderFrame(diagnosticFrame);
			presentFrame(diagnosticFrame, frameStart);
		}
		logger.info("Engine: render loop exited at frame {}.", frameCount);
	}

	/* ---------- 流程 12：输出统计结果 ---------- */
	void Engine::reportSummary()
	{
		logger.info("Average Frame Rate: {:.2f} FPS", averageFrameRate);
		logger.info(
			"FrameRate difference from target: {:.2f} %",
			(averageFrameRate - setting.FrameRate) / setting.FrameRate * 100.0f
		);

		logger.info(
			"FrameRate stability (CV): {:.2f} %",
			frameRateStability
		);
	}
}
