#pragma once

#include "image.h"

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
	gl::Texture loadTexture_stb(const std::string& path, bool sRGB, bool flip) {
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

		const GLenum internalFormat = sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
		return gl::Texture::create2D(
			width,
			height,
			data.get(),
			gl::Texture::SamplerInfo{},
			internalFormat,
			GL_RGBA,
			GL_UNSIGNED_BYTE
		);
	}
}
