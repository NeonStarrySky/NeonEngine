#pragma once

//-----------------标准库-----------------

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

//-----------------第三方库-----------------

#include <glad.h>

#include <stb/stb_image.h>

//-----------------neon-----------------

#include "gl/texture.h"

namespace neon::graphics {

	/// <summary>
	/// 使用stb_image库加载纹理，并创建OpenGL纹理对象。
	/// </summary>
	/// <param name="path">文件路径</param>
	/// <returns>纹理对象</returns>
	static gl::Texture loadTexture_stb(const std::string& path);

	/*void temp() {
		while (true) {
			std::string input;
			std::cout << "Please enter image's path.";
			std::cin >> input;
			try {
				auto tex = loadTexture_stb(input);
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
	}*/
}