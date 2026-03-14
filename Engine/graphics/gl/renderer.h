#pragma once

#include "graphics/gl/shader.h"
#include "mesh.h"

#include <glad.h>
#include <glm/fwd.hpp>

namespace neon::graphics::gl
{
	class Renderer
	{
	public:

		static void Draw(const Mesh& mesh, const Shader& shader, const glm::vec4& color, GLfloat t, const glm::vec3& cameraFront, const glm::vec3& cameraPosition, const glm::vec3& worldUp);

	};

}