#include "graphics/window_info.h"

// glad must be included before GLFW (and before any header that might pull in
// the system OpenGL headers). Use the correct include path for glad.

#ifdef _DEBUG
#include "tool/conhost.h"
#include "window.h"
#endif // _DEBUG

#include <glad.h>
#include <GLFW/glfw3.h>

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
	std::optional<Window> Window::initGlad()
	{

		//0. 如果 GLAD 已经初始化，直接返回
		if (Window::GladInitialized) // GLAD 已经初始化，直接返回
		{
#ifdef _DEBUG
			tool::setColor(FOREGROUND_RED | FOREGROUND_GREEN);
			std::cout << "GLAD is already initialized, skipping initialization.\n\n";
			tool::setColor();
#endif
			return std::nullopt;
		}

		// 1. 初始化 GLFW
		if (!glfwInit())
		{
			throw std::runtime_error("Failed to initialize GLFW");
		}

#ifdef _DEBUG
		tool::setColor(BACKGROUND_GREEN | BACKGROUND_INTENSITY);
		std::cout << "GLFW initialization successfully！\n\n";
		tool::setColor();
#endif

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
		Window tempWindowPtr(info);
		//GLWindow::GladInitialized = false;

		// 检查窗口创建是否成功
		if (!tempWindowPtr.is_good())
		{
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window");
		}

		// 4. 绑定上下文
		glfwMakeContextCurrent(
			tempWindowPtr.
			windowPtr.get()
		);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))// 初始化 GLAD
		{
			throw std::runtime_error("Failed to initialize GLAD");
		}

		//glGetIntegerv(GL_MAJOR_VERSION, 4);
		//glGetIntegerv(GL_MINOR_VERSION, 6);

#ifdef _DEBUG
		// 打印 OpenGL 信息
		std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";
		std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
		std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
		std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n\n";

		tool::setColor(BACKGROUND_GREEN | BACKGROUND_INTENSITY);
		std::cout << "GLAD initialized successfully.\n\n";
		tool::setColor();
#endif
		Window::GladInitialized = true;
		return tempWindowPtr; // 返回临时窗口对象，调用者可以选择销毁它
	}

	//这个函数接收窗口构建信息，创建上下文并且绑定在窗口上
	Window::Window(const WindowInfo& info, const std::source_location& loc) : info(info)
	{
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

#ifdef _DEBUG
		tool::setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		std::cout
			<< "Creating GLWindow Class with\n>>> pointer <" << std::to_address(windowPtr) << "> !\n"
			<< ">>> title <" << info.title << ">\n\n";
		tool::setColor();
#endif // DEBUG

	}

	Window::~Window()//全部资源都由智能指针管理，GLFW窗口会在智能指针析构时自动销毁，因此这里不需要手动调用 glfwDestroyWindow 或 glfwTerminate
	{
#ifdef _DEBUG
		if (std::to_address(windowPtr)) {
			tool::setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
			std::cout
				<< "Destroying GLWindow Class with\n>>> pointer <" << std::to_address(windowPtr) << "> !\n"
				<< ">>> title <" << info.title << ">\n\n";
			tool::setColor();
		}

#endif // DEBUG

	}

	Window::Window(Window&& other) noexcept :
		windowPtr(std::move(other.windowPtr)),	// 移动窗口资源(windowPtr是智能指针)
		info(other.info)						// 直接复制窗口信息（WindowInfo是一个简单的结构体，支持默认的移动语义）
	{
		if (this != &other) {
			other.info = WindowInfo();			// Reset the moved-from object's info
		}
	}

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

	void Window::makeContextCurrent()
	{
		glfwMakeContextCurrent(windowPtr.get());
	}

};