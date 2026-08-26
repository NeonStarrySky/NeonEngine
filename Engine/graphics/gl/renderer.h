#pragma once

#include "graphics/gl/GLResource.hpp"
#include "mesh.h"

#include <glad/glad.h>
#include <glm/fwd.hpp>

namespace neon::graphics
{
	class Renderer
	{
		using Mesh = gl::Mesh;
		using Program = gl::Program;
	public:

		void Draw(
			const Mesh& mesh,
			const Program& program,
			const glm::vec4& color,
			const glm::mat4& model,
			const glm::mat4& view,
			const glm::mat4& projection
		);
	};

}