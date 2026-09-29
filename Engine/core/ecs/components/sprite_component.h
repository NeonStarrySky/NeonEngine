#pragma once

#include "core/setting.h"
#include "graphics/gl/mesh.h"
#include "graphics/gl/texture.h"

#include <glm/glm.hpp>

namespace neon::core::ecs {

	struct UVRect {
		glm::vec2 topLeft;
		glm::vec2 bottomRight;
	};

	struct SpriteRegion {
		int x;      // 起始横坐标
		int y;      // 起始纵坐标
		int width;  // 宽度x
		int height; // 高度y
	};
	struct SpriteComponent
	{
		using Texture = graphics::gl::Texture;
		using Mesh = graphics::gl::Mesh;

		graphics::gl::Mesh& mesh; // 关联的网格
		graphics::gl::Texture& texture;
		SpriteRegion region;    // 图集中的哪个区域
		float rotation = 0;
		glm::vec2 size;

		SpriteComponent(Mesh& mesh, Texture& texture, const SpriteRegion& region, const Setting& setting)
			:
			mesh(mesh), texture(texture), region(region),
			size(
				(float)texture.info().width / setting.windowInfo.width * 2,
				(float)texture.info().height / setting.windowInfo.height * 2
			)
		{}

		UVRect getUVCoordinates() const {
			UVRect uv;
			uv.topLeft = glm::vec2(static_cast<float>(region.x) / texture.info().width,
				static_cast<float>(region.y) / texture.info().height);
			uv.bottomRight = glm::vec2(static_cast<float>(region.x + region.width) / texture.info().width,
				static_cast<float>(region.y + region.height) / texture.info().height);
			return uv;
		}
	};
}