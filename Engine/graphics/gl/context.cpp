#include "context.h"

#include <stdexcept>
#include <utility>

namespace neon::graphics::gl
{
	struct Context::GlfwRuntime
	{
		GlfwRuntime()
		{
			if (!glfwInit()) {
				throw std::runtime_error("Failed to initialize GLFW");
			}
		}

		~GlfwRuntime() { glfwTerminate(); }
		GlfwRuntime(const GlfwRuntime&) = delete;
		GlfwRuntime& operator=(const GlfwRuntime&) = delete;
	};

	std::shared_ptr<Context::GlfwRuntime> Context::acquireRuntime()
	{
		// GLFW is process-wide. An overlapping Context must not terminate it
		// while another Context still owns windows. GLFW operations stay on
		// the application thread, including acquisition and destruction here.
		static std::weak_ptr<GlfwRuntime> sharedRuntime;
		auto instance = sharedRuntime.lock();
		if (!instance) {
			instance = std::make_shared<GlfwRuntime>();
			sharedRuntime = instance;
		}
		return instance;
	}

	Context::~Context() = default;

	int Context::createWindow(const WindowInfo& info)
	{
		if (!runtime) {
			runtime = acquireRuntime();
		}

		Window candidate(info, logger);
		candidate.makeContextCurrent();
		if (windows.empty()) {
			// Load functions with a real context current. Each Context loads again,
			// avoiding a process-wide "initialized" flag surviving GLFW shutdown.
			if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
				throw std::runtime_error("Failed to initialize GLAD");
			}
			glVersion = GLVersion;
			logger.info("OpenGL Version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
			logger.info("GLSL Version: {}", reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
			logger.info("Renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
			logger.info("Vendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		}

		windows.emplace_back(std::move(candidate));
		return static_cast<int>(windows.size() - 1);
	}

	void Context::makeCurrent() const noexcept
	{
		if (auto* window = getCurrentWindowPtr()) {
			glfwMakeContextCurrent(window);
		}
	}

	GLFWwindow* Context::getWindowPtr(int index) const noexcept
	{
		if (index < 0 || static_cast<std::size_t>(index) >= windows.size()) {
			return nullptr;
		}
		return windows[index].getGLFWwindow();
	}

	GLFWwindow* Context::getCurrentWindowPtr() const noexcept
	{
		return currentWindowIndex < windows.size()
			? windows[currentWindowIndex].getGLFWwindow() : nullptr;
	}

	Window& Context::getWindow(int index)
	{
		if (index < 0 || static_cast<std::size_t>(index) >= windows.size()) {
			throw std::out_of_range("Window index out of range");
		}
		return windows[index];
	}

	Window& Context::getCurrentWindow() { return windows.at(currentWindowIndex); }
}
