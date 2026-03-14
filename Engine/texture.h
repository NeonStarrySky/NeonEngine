#pragma once

#include <GLFW/glfw3.h>

namespace neon::graphics::gl {
	struct TextureDeleter {
		void operator()(GLuint id)noexcept {
			glDeleteTextures(id);
		}
	};
	class Texture {

	};
}