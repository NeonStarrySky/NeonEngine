#pragma once

//-----------------标准库-----------------

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

//-----------------第三方库-----------------

#include <glad/glad.h>

#include <stb_image.h>

//-----------------neon-----------------

#include "gl/texture.h"

namespace neon::graphics {

	/// <summary>
	/// 使用stb_image库加载纹理，并创建OpenGL纹理对象。
	/// </summary>
	/// <param name="path">文件路径</param>
	/// <returns>纹理对象</returns>
	gl::Texture loadTexture_stb(const std::string& path, bool sRGB = false, bool flip = true);
}