#include "graphics/window_info.h"

// glad must be included before GLFW (and before any header that might pull in
// the system OpenGL headers). Use the correct include path for glad.

#include"core/logger.h"
#include"window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<spdlog/spdlog.h>

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <source_location>
#include <stdexcept>
#include <string>
#include <utility>

namespace neon::graphics::gl
{
	//初始化并且返回临时窗口，使得之后可以删掉它
	std::optional<Window> initGlad(Window::GLInfo _GLInfo, core::Logger& logger)
	{
		static bool gladInitialized = false; // 静态变量，确保 GLAD 只初始化一次
		//0. 如果 GLAD 已经初始化，直接返回
		if (gladInitialized) // GLAD 已经初始化，直接返回
		{
			logger.info("GLAD has already been initialized. Skipping re-initialization.");

			return std::nullopt;
		}

		// 1. 初始化 GLFW
		if (!glfwInit())
		{
			throw std::runtime_error("Failed to initialize GLFW");
		}

		logger.info("GLFW initialized successfully.");

		// 2. 配置 OpenGL 4.6 Core Profile
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, _GLInfo.glMajorVersion);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, _GLInfo.glMinorVersion);
		glfwWindowHint(GLFW_OPENGL_PROFILE, _GLInfo.coreProfile);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, _GLInfo.forwardCompatible);

		// 可选：窗口是否显示（离屏可设 false）
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);//这是临时窗口，所以不显示

		// 3. 创建窗口
		WindowInfo info{
			.width = 1,
			.height = 1,
			.title = "Temporary GLAD Initialization Window,but how do you see this?"
		};

		//GLWindow::GladInitialized = true;
		Window tempWindowPtr(info, logger);
		//GLWindow::GladInitialized = false;

		// 检查窗口创建是否成功
		if (!tempWindowPtr.is_good())
		{
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window");
		}

		// 4. 绑定上下文
		glfwMakeContextCurrent(
			tempWindowPtr.getGLFWwindow()
		);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))// 初始化 GLAD
		{
			throw std::runtime_error("Failed to initialize GLAD");
		}

		//glGetIntegerv(GL_MAJOR_VERSION, 4);
		//glGetIntegerv(GL_MINOR_VERSION, 6);


		// 打印 OpenGL 信息
		logger.info("OpenGL Version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
		logger.info("GLSL Version: {}", reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
		logger.info("Renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
		logger.info("Vendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		logger.info("GLAD initialized successfully.");

		gladInitialized = true;
		return tempWindowPtr; // 返回临时窗口对象，调用者可以选择销毁它
	}

	//这个函数接收窗口构建信息，创建上下文并且绑定在窗口上
	Window::Window(const WindowInfo& info, core::Logger& logger, const std::source_location& loc)
		:
		info(info), logger(logger)
	{
		//assert(logger != nullptr && "Logger pointer must not be null");

		logger.debug("Creating GLFW window with title: {}", info.title);

		// 2. 配置 OpenGL 4.6 Core Profile
		glfwDefaultWindowHints();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, _GLInfo.glMajorVersion);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, _GLInfo.glMinorVersion);
		glfwWindowHint(GLFW_OPENGL_PROFILE, _GLInfo.coreProfile);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, _GLInfo.forwardCompatible);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);//这是临时窗口，所以不显示

		windowPtr.reset(glfwCreateWindow(info.width, info.height, info.title.c_str(), nullptr, nullptr));

		// 检查窗口创建是否成功
		if (!windowPtr)//智能指针不需要处理
		{
			std::string msg = "Failed to create GLFW window with title: " + info.title;
			throw std::runtime_error(
				msg +
				" | File: " + loc.file_name() +
				" | Line: " + std::to_string(loc.line())
			);
		}


		logger.info("GLFW window created successfully with title: {}", info.title);

	}

	Window::~Window()//全部资源都由智能指针管理，GLFW窗口会在智能指针析构时自动销毁，因此这里不需要手动调用 glfwDestroyWindow 或 glfwTerminate
	{
		//if (std::to_address(windowPtr)) {
		//	logger.info("Destroying GLFW window with title: {}", info.title);
		//	logger.info("Destroying GLFW window with pointer: {}", static_cast<const void*>(std::to_address(windowPtr)));
		//}
	}

	//Window::Window(Window&& other) noexcept :
	//	windowPtr(std::move(other.windowPtr)),	// 移动窗口资源(windowPtr是智能指针)
	//	info(other.info)						// 直接复制窗口信息（WindowInfo是一个简单的结构体，支持默认的移动语义）
	//{
	//	if (this != &other) {
	//		other.info = WindowInfo();			// Reset the moved-from object's info
	//	}
	//}

	// 渲染循环由外部控制
	bool Window::shouldClose() const
	{
		return glfwWindowShouldClose(windowPtr.get());
	}

	void Window::swapBuffers()
	{
		glfwSwapBuffers(windowPtr.get());
	}

	void Window::pollEvents()
	{
		glfwPollEvents();
	}

	void Window::processInput()
	{
		if (glfwGetKey(windowPtr.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(windowPtr.get(), true);
	}

	//绑定上下文
	void Window::makeContextCurrent()
	{
		glfwMakeContextCurrent(windowPtr.get());
	}

};