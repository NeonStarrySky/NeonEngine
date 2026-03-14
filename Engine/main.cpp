

#include "Engine.h"
#include <glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>// 这里展开了stb_image.h的实现，包含了stbi_load和stbi_image_free函数的定义

#include <filesystem>  // C++17
#include <iostream>
#include <stdexcept>
#include <string>
#include <string>
#include <vector>

static GLuint loadTexture(const char* path)
{
	// 检测空路径
	if (path == nullptr) {
		throw std::invalid_argument("Texture path is null");
	}

	// 检测空字符串
	if (std::strlen(path) == 0) {
		throw std::invalid_argument("Texture path is empty");
	}

	// 检测文件是否存在
	if (!std::filesystem::exists(path)) {
		throw std::runtime_error(
			std::string("Texture file does not exist: ") + path
		);
	}

	// 检测是否为常规文件（而非目录）
	if (!std::filesystem::is_regular_file(path)) {
		throw std::runtime_error(
			std::string("Texture path is not a regular file: ") + path
		);
	}

	int w, h, ch;
	unsigned char* data = stbi_load(path, &w, &h, &ch, 0);

	// 检测 stbi_load 是否成功加载
	if (data == nullptr) {
		const char* reason = stbi_failure_reason();
		throw std::runtime_error(
			std::string("Failed to load texture: ") + path +
			" (Reason: " + (reason ? reason : "unknown") + ")"
		);
	}

	// 检测图像尺寸有效性
	if (w <= 0 || h <= 0) {
		stbi_image_free(data);
		throw std::runtime_error(
			std::string("Invalid texture dimensions (") +
			std::to_string(w) + "x" + std::to_string(h) +
			"): " + path
		);
	}

	GLuint tex;
	glGenTextures(1, &tex);

	if (tex == 0) {
		stbi_image_free(data);
		throw std::runtime_error("Failed to generate OpenGL texture object");
	}

	glBindTexture(GL_TEXTURE_2D, tex);

	GLenum format = (ch == 4) ? GL_RGBA : GL_RGB;

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		format,
		w,
		h,
		0,
		format,
		GL_UNSIGNED_BYTE,
		data
	);

	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	return tex;
}

int main(int argc, char* argv[]) {


	using neon::core::Engine;

	Engine engine;

	while (true) {
		std::string input;
		std::cout << "Please enter image's path.";
		std::cin >> input;
		try {
			GLuint tex = loadTexture(input.c_str());
			// 使用纹理...
		}
		catch (const std::invalid_argument& e) {
			// 处理参数错误（空路径等）
			std::cerr << "Invalid argument: " << e.what() << std::endl;
		}
		catch (const std::runtime_error& e) {
			// 处理运行时错误（文件不存在、加载失败等）
			std::cerr << "Runtime error: " << e.what() << std::endl;
		}
		catch (const std::exception& e) {
			// 捕获其他标准异常
			std::cerr << "Error: " << e.what() << std::endl;
		}

	}

	return 0;
	engine.run();
	/*neon::gameplay::World world;
	std::string command;
	while (true)
	{
		std::cout << "Enter command (create(c)/destroy(d)/update/exit): ";
		std::cin >> command;
		if (command == "c") {
			auto entity = world.createEntity();
			std::cout << "Created entity: " << entity << std::endl;
		}
		else if (command == "d") {
			std::cout << "Enter entity ID to destroy: ";
			uint32_t entityId;
			std::cin >> entityId;
			world.destroyEntity(entityId);
			std::cout << "Destroyed entity: " << entityId << std::endl;
		}
		else if (command == "update") {
			world.update(0.016); // 假设每帧更新16ms
			std::cout << "Updated world." << std::endl;
		}
		else if (command == "exit") {
			break;
		}
		else {
			std::cout << "Unknown command." << std::endl;
		}
	}*/


}