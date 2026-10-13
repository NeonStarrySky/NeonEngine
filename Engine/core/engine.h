#pragma once

#include "core/ecs/entity_manager.h"
#include "core/ecs/system/physics_system.h"
#include "core/ecs/system/sprite_system.h"
#include "graphics/frameRateController.h"
#include "graphics/gl/renderer.h"
#include "graphics/gl/shader_manager.h"
#include "graphics/gl/window.h"
#include "input_system.h"
#include "logger.h"
#include "setting.h"

#include <array>
#include <chrono>
#include <cstddef>
#include <deque>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

namespace neon::core
{
	class Engine
	{
		using Window = graphics::gl::Window;
		using WindowInfo = graphics::WindowInfo;
		using DiagnosticClock = std::chrono::steady_clock;

		enum class SpriteRenderMode
		{
			Individual,
			Array,
			TextureGrouped
		};

		static constexpr int sampleCount = 60;
		static constexpr int diagnosticIntervalFrames = 180;
		static constexpr std::size_t entityCount = 10000 * 10;
		static constexpr std::array<int, 3> renderModeKeys{ GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3 };
		static constexpr std::array<const char*, 3> renderModeNames{
			"Individual Sprite Renderer",
			"Sprite Array Renderer",
			"Texture Grouped Sprite Renderer"
		};

		// Dependencies are declared before the systems and resources that use them.
		Logger logger{};
		Setting setting;
		ecs::EntityManager entityManager{ logger };
		ecs::ComponentManager componentManager;
		ecs::PhysicsSystem physicsSystem{ logger, setting, componentManager };
		ecs::SpriteSystem spriteSystem{ logger, setting, componentManager };

		graphics::gl::ShaderManager shaderManager{ logger };
		std::vector<Window> windows;
		std::size_t currentWindowIndex = 0;
		InputSystem input_system;
		graphics::FrameRateController frameRateController;
		graphics::Renderer renderer;
		graphics::gl::Program program;

		std::optional<graphics::gl::Mesh> mesh;
		std::deque<graphics::gl::Texture> entityTextures;
		std::mt19937 randomEngine{ std::random_device{}() };

		graphics::gl::UniformBuffer shareUBO;
		glm::mat4 view{ 1.0f };
		glm::mat4 projection{ 1.0f };
		SpriteRenderMode renderMode{ SpriteRenderMode::Array };
		std::array<bool, renderModeKeys.size()> renderModeKeyWasPressed{};
		DiagnosticClock::time_point modeIntervalStartedAt{};
		std::size_t modeIntervalFrameCount = 0;
		int frameCount = 0;

		std::deque<float> frameRates;
		float averageFrameRate = 0.0f;
		float frameRateStdDev = 0.0f;
		float frameRateStability = 0.0f;
		gladGLversionStruct glVersion{};
		bool glfwInitialized = false;
		bool initialized = false;

		void startup(const char* executablePath);
		void initWindow();
		bool initGraphics();
		void initScene();
		void initRenderState();
		void updateFrameInput(bool diagnosticFrame);
		void updateFrameSimulation(bool diagnosticFrame);
		void renderFrame(bool diagnosticFrame);
		void updateFrameRateStatistics(float currentFrameRate);
		void presentFrame(bool diagnosticFrame, DiagnosticClock::time_point frameStart);
		void runMainLoop();
		void reportSummary();

	public:
		Engine() = default;
		~Engine();
		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		// Initializes the window, shaders, sprite scene and render state.
		// Returns false when shader initialization fails.
		bool init(const char* executablePath = "");
		void run();

		// Returns the zero-based index of the newly created window.
		int creatWindow(WindowInfo info = WindowInfo());
		InputSystem& getInputSystem() { return input_system; }

		GLFWwindow* getGLFWWindowPtr(int i) const
		{
			if (i < 0 || static_cast<std::size_t>(i) >= windows.size()) {
				return nullptr;
			}
			return windows[i].getGLFWwindow();
		}

		GLFWwindow* getCurrentGLFWWindowPtr() const
		{
			return currentWindowIndex < windows.size()
				? windows[currentWindowIndex].getGLFWwindow() : nullptr;
		}

		Window& getWindow(int i)
		{
			if (i < 0 || static_cast<std::size_t>(i) >= windows.size()) {
				throw std::out_of_range("Window index out of range");
			}
			return windows[i];
		}

		Window& getCurrentWindow() { return windows.at(currentWindowIndex); }
		gladGLversionStruct getGLVersion() const { return glVersion; }
	};
}
