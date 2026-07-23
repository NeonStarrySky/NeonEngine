#pragma once

//-----------------标准库-----------------

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

//-----------------第三方库-----------------

#include <glad/glad.h>
//展开定义
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//-----------------neon-----------------

#include "gl/texture.h"

namespace neon::graphics {
	static gl::Texture loadTexture_stb(const std::string& path, bool sRGB = false, bool flip = true) {
		// 1. flip guard（线程安全）
		/*struct FlipGuard {
			int old;
			FlipGuard(bool f) { old = stbi_set_flip_vertically_on_load_thread(f); }
			~FlipGuard() { stbi_set_flip_vertically_on_load_thread(old); }
		} flipGuard(flip);*/

		stbi_set_flip_vertically_on_load_thread(flip);
		// 2. 加载（统一为 RGBA）
		int width, height, channels;
		unsigned char* raw = stbi_load(path.c_str(), &width, &height, &channels, 4);
		if (!raw) {
			throw std::runtime_error(
				"Failed to load image: " + path +
				"\nReason: " + stbi_failure_reason()
			);
		}

		using ImagePtr = std::unique_ptr<unsigned char, void(*)(void*)>;
		ImagePtr data(raw, stbi_image_free);

		// 3. 像素对齐（避免 RGB 等错位问题）
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		GLenum internalFormat = sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
		GLenum format = GL_RGBA;

		// 4. 创建纹理
		gl::Texture::Info info;
		info.target = GL_TEXTURE_2D;
		info.width = width;
		info.height = height;

		gl::Texture texture(info);

		// 5. 分配 + 上传
		texture.allocateStorage(1, internalFormat, width, height);
		texture.uploadData(0, format, GL_UNSIGNED_BYTE, data.get());

		// ❌ 不做任何参数设置
		// ❌ 不生成 mipmap

		return texture;
	}
}