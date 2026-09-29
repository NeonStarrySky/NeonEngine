#pragma once

#include "graphics/gl/GLResource.hpp"
#include "mesh.h"

#include "core/ecs/components/sprite_component.h"

#include <core/ecs/components/physics_component.h>
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

		void Draw(
			const core::ecs::SpriteComponent& sprite,
			const core::ecs::PhysicsComponent& physic,
			const Program& program,
			const glm::mat4& view,
			const glm::mat4& projection
		);
	};

}