#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "core/logger.h"

#include <stdexcept>

namespace neon::graphics::gl
{
	// Borrows an Engine-owned window; never creates or destroys windows.
	// GLFW ties the actual OpenGL context lifetime to that window.
	class Context
	{
		GLFWwindow* window;
		gladGLversionStruct version{};

	public:
		// A scoped current-context binding. The previous window must stay alive
		// until this scope ends. Use on the thread that runs rendering.
		class Binding
		{
			GLFWwindow* previous;

		public:
			explicit Binding(GLFWwindow* window) noexcept
				: previous(glfwGetCurrentContext())
			{
				glfwMakeContextCurrent(window);
			}

			~Binding()
			{
				glfwMakeContextCurrent(previous);
			}

			Binding(const Binding&) = delete;
			Binding& operator=(const Binding&) = delete;
			Binding(Binding&&) = delete;
			Binding& operator=(Binding&&) = delete;
		};

		Context(GLFWwindow* window, core::Logger& logger) : window(window)
		{
			if (!window) {
				throw std::invalid_argument("OpenGL context requires a window");
			}
			// Restore the caller's binding even when GLAD initialization throws.
			auto binding = bind();
			if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
				throw std::runtime_error("Failed to initialize GLAD");
			}
			version = GLVersion;
			logger.info("OpenGL Version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
			logger.info("GLSL Version: {}", reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
			logger.info("Renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
			logger.info("Vendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		}

		~Context()
		{
			// The Engine-owned window still exists here. Release only our binding;
			// the window's RAII owner destroys the actual OpenGL context afterward.
			if (glfwGetCurrentContext() == window) {
				glfwMakeContextCurrent(nullptr);
			}
		}

		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;
		Context(Context&&) = delete;
		Context& operator=(Context&&) = delete;

		Binding bind() const noexcept
		{
			return Binding(window);
		}

		void makeCurrent() const noexcept
		{
			glfwMakeContextCurrent(window);
		}

		gladGLversionStruct getGLVersion() const noexcept
		{
			return version;
		}
	};
}
