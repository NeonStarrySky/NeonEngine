#include "engine.h"

#include "graphics/frameRateController.h"

#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <iostream>
#include <stdexcept>
#include<vector>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "logger.h"
#include <graphics/gl/GLResource.hpp>
#include <timer.h>

namespace neon::core
{

	void Engine::init() {

		logger.setLogType(Logger::LogType::async);
		logger.setLogLevel(Logger::LogLevel::debug);

		logger.debug("Engine initialization started.");

		//局部作用域，确保临时窗口在创建正式窗口之后被销毁
		{
			auto temp = graphics::gl::initGlad(Window::GLInfo(), logger); // Initialize GLAD to load OpenGL function pointers
			creatWindow(WindowInfo{});
			getCurrentWindow().makeContextCurrent();
		}

		auto* windowPtr = getCurrentWindow().getGLFWwindow();

		glfwSetWindowUserPointer(windowPtr, this);

		input_system.init(windowPtr); // Initialize the input system with the GLFW window

		logger.info("GLFW and GLAD initialized successfully.");
		logger.info("OpenGL version: {}.{}", glVersion.major, glVersion.minor);
		logger.info("GLSL version: {}", *glGetString(GL_SHADING_LANGUAGE_VERSION));

		logger.info("Initialize shader_manager and loading shaders...");
		shader_manager.init(); // Initialize the shader manager
		shader_manager.loadShaders({// Load and compile shaders from specified file paths and types
			{"assets/shaders/vertex_shader.glsl", GL_VERTEX_SHADER},
			{"assets/shaders/aivs.glsl", GL_VERTEX_SHADER},
			{ "assets/shaders/fragment_shader.glsl", GL_FRAGMENT_SHADER }
			});
		logger.info("Initialize shader_manager and loading shaders successful.");

		logger.info("Engine initialized successful.");

	}

	Engine::Engine() :
		logger("log.txt"),
		input_system(),
		shader_manager(logger)
	{}

	Engine::~Engine()//raii已经保证资源的正确释放，这里不需要手动清理窗口资源
	{
		logger.debug("Engine destructor called. Cleaning up resources...");

		glfwTerminate();

	}

	int Engine::creatWindow(WindowInfo info)
	{
		windows.emplace_back(Window(info, logger)); // Create a new window and add it to the list of windows
		return windows.size();
	}
	void Engine::run()
	{

		//链接着色器
		shader_manager.linkPrograms({ "assets/shaders/vertex_shader.glsl", "assets/shaders/fragment_shader.glsl" });
		graphics::gl::Program program;
		try {
			program = shader_manager.buildShader();
		}
		catch (const std::runtime_error& e)
		{
			logger.error("Shader build error: {}", e.what());

			return;
		}
		program.use();
		glEnable(GL_DEPTH_TEST);

		using neon::graphics::gl::Vertex;

		// 顶点数据布局：Position (x,y,z), Normal (x,y,z), TexCoords (u,v)
		std::vector<Vertex> vertices = {
			{{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
			{{ 0.2f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
			{{ 0.2f,  0.2f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
			{{1.0f,  0.2f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
			{{1.0f, 1.0f,  0.2f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
			{{ 0.2f, 1.0f,  0.2f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
			{{ 0.2f,  0.2f,  0.2f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
			{{1.0f,  0.2f,  0.2f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}
		};

		// 索引数据 (按三角形拆分)
		std::vector<unsigned int> indices = {
			// 背面 (z = -0.5)
			0, 1, 2,
			2, 3, 0,
			// 正面 (z = 0.5)
			4, 5, 6,
			6, 7, 4,
			// 左面 (x = -0.5)
			0, 3, 7,
			7, 4, 0,
			// 右面 (x = 0.5)
			1, 5, 6,
			6, 2, 1,
			// 底面 (y = -0.5)
			0, 4, 5,
			5, 1, 0,
			// 顶面 (y = 0.5)
			3, 2, 6,
			6, 7, 3
		};

		Mesh mesh(vertices, indices);

		logger.info("Starting main loop. Press ESC to exit.");

		int frameCount = 0;
		Timer timer;

		glm::vec3 cameraPos = glm::vec3(2.0f, 2.0f, 2.0f);
		glm::vec3 cameraFront = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
		glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

		glm::mat4 model = glm::mat4(1.0f);

		float angle = -0.577f;

		model = glm::translate(
			model,
			glm::vec3(0, 0, 0)
		);

		model = glm::rotate(
			model,
			angle,
			glm::vec3(0, 1, 0)
		);

		glm::mat4 view = glm::lookAt(
			cameraPos,
			cameraPos + cameraFront,
			worldUp
		);

		auto width = getCurrentWindow().getWindowInfo().width;
		auto height = getCurrentWindow().getWindowInfo().height;

		glm::mat4 projection = glm::perspective(
			glm::radians(45.0f),
			width / (float)height,
			0.1f,
			100.0f
		);

		graphics::FrameRateController frameRateController;
		frameRateController.setFrameRate(120);

		//gameplay::World world;

		auto forward = glm::normalize(cameraFront);
		auto right = glm::normalize(glm::cross(forward, worldUp));
		auto up = glm::normalize(glm::cross(right, forward));

		while (!glfwWindowShouldClose(windows[0].getGLFWwindow()))
		{
			//事件处理流程
			getWindow(0).pollEvents();

			if (input_system.keyPressed(Key::Escape))//如果按下了ESC键，关闭窗口
			{
				glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
				break; // 退出循环，结束程序
			}
			/*if (input_system.keyPressed(Key::Space)) {
				std::cout << "input_system.keyPressed(Key::Space)\n";
			}
			if (input_system.keyPressing(Key::Space)) {
				std::cout << "input_system.keyPressing(Key::Space)\n";
			}*/

			timer.tick();
			float deltaTime = static_cast<float> (timer.deltaTime()); // 获取当前帧的时间增量
			float k = 1.0f;
			if (input_system.keyPressed(Key::W))
			{
				cameraPos += forward * deltaTime * k; // 按W键前进

			}
			else if (input_system.keyPressed(Key::S))
			{
				cameraPos -= forward * deltaTime * k; // 按S键后退

			}
			else if (input_system.keyPressed(Key::A))
			{
				cameraPos -= right * deltaTime * k; // 按A键左移

			}
			else if (input_system.keyPressed(Key::D))
			{
				cameraPos += right * deltaTime * k; // 按D键右移

			}
			else if (input_system.keyPressed(Key::LeftShift))
			{
				cameraPos += up * deltaTime * k; // 按左Shift键上移

			}
			else if (input_system.keyPressed(Key::LeftCtrl))
			{
				cameraPos -= up * deltaTime * k; // 按左Ctrl键下移

			}

			//world.update(deltaTime);
			//input_system.swap(); // Update the input system state

			//绘制流程
			// 清屏
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			renderer.Draw(mesh, program, glm::vec4(1.0f, 0.5f, 0.2f, 1.0f), model, view, projection);

			getWindow(0).swapBuffers();


			frameRateController.checkAndWait();
			frameCount++;
			if (frameCount % setting.FrameRate == 0) // 每60帧输出一次FPS
			{
				using std::round;
				frameCount = 0;
				std::cout << "Frame rate <" << frameRateController.getActualFrameRate() << "> \n";
				std::cout << 'x' << cameraPos.x << 'y' << cameraPos.y << 'z' << cameraPos.z << '\n';
				//std::cout << timer.deltaTime() << " seconds elapsed. ";
				//std::cout << frameCount << std::endl;
				//double fps = 1.0 / timer.deltaTime(); // Calculate FPS based on the delta time
				//std::cout << "FPS: " << fps << std::endl; // Output the FPS to the console
			}
		}


	}
}
