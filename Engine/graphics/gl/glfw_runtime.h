#pragma once

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>
#include <memory>
#include <stdexcept>

namespace neon::graphics::gl
{
	// Process-wide GLFW lifetime. Windows must be destroyed before this owner.
	// Acquire and release on the application thread, as required by GLFW.
	class GlfwRuntime
	{
		struct State
		{
			State()
			{
				if (!glfwInit()) {
					throw std::runtime_error("Failed to initialize GLFW");
				}
			}

			~State()
			{
				glfwTerminate();
			}

			State(const State&) = delete;
			State& operator=(const State&) = delete;
		};

		static std::shared_ptr<State> acquireState()
		{
			// The last live runtime releases GLFW; one Engine cannot terminate it
			// while another Engine still owns windows.
			static std::weak_ptr<State> sharedState;
			auto instance = sharedState.lock();
			if (!instance) {
				instance = std::make_shared<State>();
				sharedState = instance;
			}
			return instance;
		}

		std::shared_ptr<State> state;

	public:
		GlfwRuntime() : state(acquireState()) {}
		~GlfwRuntime() = default;
		GlfwRuntime(const GlfwRuntime&) = delete;
		GlfwRuntime& operator=(const GlfwRuntime&) = delete;
		GlfwRuntime(GlfwRuntime&&) = delete;
		GlfwRuntime& operator=(GlfwRuntime&&) = delete;
	};
}
