#pragma once

//resources
#include "graphics/gl/GLResource.hpp"
#include "graphics/gl/mesh.h"
#include "graphics/gl/window.h"
#include "graphics/window_info.h"

//submodules
#include "graphics/gl/renderer.h"
#include "graphics/gl/shader_manager.h"
#include "input_system.h"//glfw
#include "logger.h"
#include "timer.h"

//glad and glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//standard library
#include <memory>
#include <stdexcept>
#include <vector>

namespace neon::core
{
	class Engine
	{
		// 类型别名，简化代码书写
		using Window = neon::graphics::gl::Window;
		using ShaderManager = neon::graphics::gl::ShaderManager;
		using Shader = neon::graphics::gl::Shader;
		using Mesh = neon::graphics::gl::Mesh;
		using WindowInfo = neon::graphics::WindowInfo;
		using Renderer = neon::graphics::gl::Renderer;
		using InputSystem = neon::core::InputSystem;
		using Logger = neon::core::Logger;

		Window::GLInfo _GLInfo;

		struct Context {
			size_t current_window = 0;
		}context;

		gladGLversionStruct glVersion;

		//resources
		std::vector<Window> windows;
		//submodules
		InputSystem input_system;
		ShaderManager shader_manager;
		Renderer renderer;
		std::unique_ptr<Logger> logger;

	public:
		Engine();
		~Engine();

		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		/// <summary>
		/// 初始化引擎核心模块（如GLFW、OpenGL上下文等）。
		/// </summary>
		void init();

		/// <summary>
		/// 启动引擎主循环，处理事件、输入和渲染。
		/// </summary>
		void run();

		/// <summary>
		/// 创建一个新的应用程序窗口。
		/// </summary>
		/// <param name="info">窗口的配置信息，如尺寸、标题等。默认为WindowInfo的默认值。</param>
		/// <returns>成功时返回新创建窗口的索引（在windows向量中的位置），失败时返回-1。</returns>
		int creatWindow(WindowInfo info = WindowInfo());

		/// <summary>
		/// 获取引擎的输入系统引用，用于查询键盘、鼠标等输入状态。
		/// </summary>
		/// <returns>对InputSystem对象的引用。</returns>
		InputSystem& getInputSystem() { return input_system; }

		/// <summary>
		/// 根据索引获取底层GLFW窗口的原始指针。
		/// </summary>
		/// <param name="i">窗口索引。</param>
		/// <returns>指向GLFWwindow对象的指针。如果索引无效，则返回nullptr。</returns>
		GLFWwindow* getGLFWWindowPtr(int i) const
		{
			if (i < 0 || i >= static_cast<int>(windows.size())) {
				return nullptr;
			}
			return windows[i].getGLFWwindow();
		}
		/// <summary>
		/// 获取当前活动窗口的底层GLFW窗口原始指针。
		/// </summary>
		/// <returns>指向当前GLFWwindow对象的指针。</returns>
		GLFWwindow* getCurrentGLFWWindowPtr() const
		{
			return windows[context.current_window].getGLFWwindow();
		}
		/// <summary>
		/// 根据索引获取对应的Window对象（引用）。
		/// </summary>
		/// <param name="i">窗口索引。</param>
		/// <returns>对应Window对象的引用。</returns>
		/// <exception cref="std::out_of_range">当索引超出有效范围时抛出。</exception>
		Window& getWindow(int i)
		{
			if (i < 0 || i >= static_cast<int>(windows.size())) {
				throw std::out_of_range("Window index out of range");
			}
			return windows[i];
		}
		/// <summary>
		/// 获取当前活动窗口的Window对象（引用）。
		/// </summary>
		/// <returns>当前Window对象的引用。</returns>
		Window& getCurrentWindow()
		{
			return windows[context.current_window];
		}
		/// <summary>
		/// 获取当前OpenGL上下文版本的详细信息。
		/// </summary>
		/// <returns>包含OpenGL主版本号、次版本号等信息的gladGLversionStruct结构体。</returns>
		const gladGLversionStruct getGLVersion() { return glVersion; }
	};
	;
}