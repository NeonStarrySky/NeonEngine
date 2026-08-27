#pragma once

#include "core/logger.h"
#include "graphics/window_info.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>
#include <optional>
#include <source_location>
#include <string>

namespace neon::graphics::gl
{
	constexpr std::string GLenumToString(GLenum value) {
		switch (value) {
			// 图元类型
		//case GL_POINTS: return "GL_POINTS";
		case GL_POINTS: return "GL_NULL";
		case GL_LINES: return "GL_LINES";
		case GL_LINE_LOOP: return "GL_LINE_LOOP";
		case GL_LINE_STRIP: return "GL_LINE_STRIP";
		case GL_TRIANGLES: return "GL_TRIANGLES";
		case GL_TRIANGLE_STRIP: return "GL_TRIANGLE_STRIP";
		case GL_TRIANGLE_FAN: return "GL_TRIANGLE_FAN";

			// 错误代码
		case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
		case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
		case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";

			// 缓冲类型
		case GL_ARRAY_BUFFER: return "GL_ARRAY_BUFFER";
		case GL_ELEMENT_ARRAY_BUFFER: return "GL_ELEMENT_ARRAY_BUFFER";

			// 数据类型
		case GL_BYTE: return "GL_BYTE";
		case GL_UNSIGNED_BYTE: return "GL_UNSIGNED_BYTE";
		case GL_SHORT: return "GL_SHORT";
		case GL_UNSIGNED_SHORT: return "GL_UNSIGNED_SHORT";
		case GL_INT: return "GL_INT";
		case GL_UNSIGNED_INT: return "GL_UNSIGNED_INT";
		case GL_FLOAT: return "GL_FLOAT";
		case GL_DOUBLE: return "GL_DOUBLE";

			// 着色器相关
		case GL_PROGRAM: return"GL_PROGRAM";
		case GL_VERTEX_SHADER: return "GL_VERTEX_SHADER";
		case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
		case GL_GEOMETRY_SHADER: return "GL_GEOMETRY_SHADER";
		case GL_TESS_CONTROL_SHADER: return "GL_TESS_CONTROL_SHADER";
		case GL_TESS_EVALUATION_SHADER: return "GL_TESS_EVALUATION_SHADER";
		case GL_COMPUTE_SHADER: return "GL_COMPUTE_SHADER";
		case GL_COMPILE_STATUS: return "GL_COMPILE_STATUS";
		case GL_LINK_STATUS: return "GL_LINK_STATUS";

			// 纹理相关
		case GL_TEXTURE_2D: return "GL_TEXTURE_2D";

		case GL_TEXTURE_3D: return "GL_TEXTURE_3D";
		case GL_TEXTURE_MIN_FILTER: return "GL_TEXTURE_MIN_FILTER";
		case GL_TEXTURE_MAG_FILTER: return "GL_TEXTURE_MAG_FILTER";
		case GL_NEAREST: return "GL_NEAREST";
		case GL_LINEAR: return "GL_LINEAR";

			// 像素格式
		case GL_RGB: return "GL_RGB";
		case GL_RGBA: return "GL_RGBA";
		case GL_RED: return "GL_RED";
		case GL_RG: return "GL_RG";

			// 深度测试
		case GL_DEPTH_TEST: return "GL_DEPTH_TEST";
		case GL_LEQUAL: return "GL_LEQUAL";
		case GL_LESS: return "GL_LESS";

			// 面剔除
		case GL_CULL_FACE: return "GL_CULL_FACE";
		case GL_FRONT: return "GL_FRONT";
		case GL_BACK: return "GL_BACK";
		case GL_FRONT_AND_BACK: return "GL_FRONT_AND_BACK";

			// 混合
		case GL_BLEND: return "GL_BLEND";
		case GL_SRC_ALPHA: return "GL_SRC_ALPHA";
		case GL_ONE_MINUS_SRC_ALPHA: return "GL_ONE_MINUS_SRC_ALPHA";

			// 帧缓冲
		case GL_FRAMEBUFFER: return "GL_FRAMEBUFFER";
		case GL_COLOR_ATTACHMENT0: return "GL_COLOR_ATTACHMENT0";
		case GL_DEPTH_ATTACHMENT: return "GL_DEPTH_ATTACHMENT";

			// 渲染缓冲
		case GL_RENDERBUFFER: return "GL_RENDERBUFFER";
		case GL_DEPTH_COMPONENT: return "GL_DEPTH_COMPONENT";
		case GL_DEPTH24_STENCIL8: return "GL_DEPTH24_STENCIL8";

			// 状态查询
		case GL_VENDOR: return "GL_VENDOR";
		case GL_RENDERER: return "GL_RENDERER";
		case GL_VERSION: return "GL_VERSION";
		case GL_SHADING_LANGUAGE_VERSION: return "GL_SHADING_LANGUAGE_VERSION";

		default: {
			// 对于未知的值，返回十六进制表示
			char buffer[32];
			snprintf(buffer, sizeof(buffer), "GL_UNKNOWN(0x%04X)", value);
			return std::string(buffer);
		}
		}
	}



	//这个类需要windowInfo（窗口信息->类内独立的）
	//还需要GLInfo（opengl）
	class Window
	{
		friend class Engine; // 友元类，允许Engine访问Window的私有成员
	private:
		core::Logger& logger; // 日志记录器指针
	public:
		struct GLInfo {
			// 渲染配置
			int glMajorVersion = 4;								// OpenGL 主版本
			int glMinorVersion = 6;								// OpenGL 次版本
			decltype(GLFW_OPENGL_CORE_PROFILE)					// 是否使用核心模式
				coreProfile = GLFW_OPENGL_CORE_PROFILE;
			decltype(GL_TRUE)									// 是否前向兼容
				forwardCompatible = GL_TRUE;
		};

	private:
		inline static GLInfo _GLInfo;

		inline static bool GladInitialized = false; // GLAD 初始化状态

	public:


		Window(const WindowInfo&, core::Logger& logger, const std::source_location& loc = std::source_location::current());  // 构造函数
		~Window();                  // 析构函数

		Window(Window&&) = default;		   // 移动构造函数

		// 禁止复制和赋值
		Window& operator=(const Window&) = delete;
		Window(const Window&) = delete;

		// 状态查询
		bool is_good() const
		{
			return windowPtr != nullptr;
		}

		void pollEvents();          // 事件处理
		void processInput(); // 输入处理
		bool shouldClose() const;   // 状态查询
		void swapBuffers();         // 渲染操作
		void makeContextCurrent();  // 绑定上下文

		static void setGLversion(int major, int minor)
		{
			_GLInfo.glMajorVersion = major;
			_GLInfo.glMinorVersion = minor;
		}

		const WindowInfo& getWindowInfo() const
		{
			return info;
		}

		operator bool() const
		{
			return !shouldClose();
		}

		/// <summary>
		/// 获取窗口句柄（只读的指针！）
		/// </summary>
		/// <returns>返回GLFWwindowz指针</returns>
		GLFWwindow* getGLFWwindow() const
		{
			return windowPtr.get();
		}

	private:
		//资源

		WindowInfo info;
		struct GLFWwindowDeleter
		{
			void operator()(GLFWwindow* window) const
			{
				if (window)
				{
					glfwDestroyWindow(window);
				}
			}
		};
		std::unique_ptr<GLFWwindow, GLFWwindowDeleter> windowPtr;
	};

	std::optional<Window> initGlad(Window::GLInfo _GLInfo, core::Logger& logger);
}