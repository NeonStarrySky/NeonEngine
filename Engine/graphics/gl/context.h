#pragma once

#include "window.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace neon::graphics::gl
{
	// Owns GLFW lifetime and its OpenGL windows. Declare this before GPU resources
	// so they are destroyed while the windows and GLFW are still alive.
	class Context
	{
		struct GlfwRuntime;
		static std::shared_ptr<GlfwRuntime> acquireRuntime();

		core::Logger& logger;
		// Reverse member destruction closes windows before releasing GLFW.
		std::shared_ptr<GlfwRuntime> runtime;
		std::vector<Window> windows;
		std::size_t currentWindowIndex = 0;
		gladGLversionStruct glVersion{};

	public:
		explicit Context(core::Logger& logger) : logger(logger) {}
		~Context();
		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;
		Context(Context&&) = delete;
		Context& operator=(Context&&) = delete;

		int createWindow(const WindowInfo& info = WindowInfo());
		void makeCurrent() const noexcept;
		GLFWwindow* getWindowPtr(int index) const noexcept;
		GLFWwindow* getCurrentWindowPtr() const noexcept;
		Window& getWindow(int index);
		Window& getCurrentWindow();
		gladGLversionStruct getGLVersion() const noexcept { return glVersion; }
	};
}
