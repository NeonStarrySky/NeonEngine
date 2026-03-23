#pragma once

#include "graphics/gl/mesh.h"
#include "graphics/gl/renderer.h"
#include "graphics/gl/shader.h"
#include "graphics/gl/shader_manager.h"
#include "graphics/gl/window.h"
#include "graphics/window_info.h"
#include "input_system.h"//glfw
#include "timer.h"

#include <vector>



namespace neon::core
{
	class Engine
	{
		using Window = neon::graphics::gl::Window;
		using ShaderManager = neon::graphics::gl::ShaderManager;
		using Shader = neon::graphics::gl::Shader;
		using Mesh = neon::graphics::gl::Mesh;
		using WindowInfo = neon::graphics::WindowInfo;

		Window::GLInfo _GLInfo;

		static gladGLversionStruct glVersion;

		//管理器及组件
		std::vector<Window> windows;
		InputSystem input_system;
		ShaderManager shader_manager;
		neon::graphics::gl::Renderer renderer;
	public:
		Engine();
		~Engine();
		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		void init();
		void run();

		int creatWindow(WindowInfo info);


		InputSystem& getInputSystem() { return input_system; }
		GLFWwindow* getGLFWWindowPtr(int i) const
		{
			if (i < 0 || i >= static_cast<int>(windows.size())) {
				return nullptr;  // 索引越界保护
			}
			return windows[i].getGLFWwindow();  // 用 operator[] 避免重复检查
		}
		Window& getGLFWWindow(int i)
		{
			if (i < 0 || i >= static_cast<int>(windows.size())) {
				throw std::out_of_range("Window index out of range");
			}
			return windows[i];  // 用 operator[] 避免重复检查
		}
		static const gladGLversionStruct getGLVersion() { return glVersion; }
	};
	;
}