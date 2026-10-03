#pragma once

#include "graphics/gl/GLResource.hpp"
#include "mesh.h"

#include "core/ecs/components/sprite_component.h"

#include "core/ecs/components/physics_component.h"
#include <cassert>
#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

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
		template<std::ranges::range SpriteRange, std::ranges::range PhysicsRange>
		void DrawSpriteArray(
			SpriteRange&& sprites,
			PhysicsRange&& physics,
			const Program& program,
			gl::UniformBuffer& shareUBO,//这个UBO应该是申请好的，里面存放了view和projection矩阵
			const glm::mat4& view,
			const glm::mat4& projection
		)
		{
			if constexpr (std::ranges::sized_range<SpriteRange> && std::ranges::sized_range<PhysicsRange>)
			{
				assert(std::ranges::size(sprites) == std::ranges::size(physics));
			}
			assert(shareUBO.isValid());

			program.use();
			shareUBO.update(0, sizeof(glm::mat4), glm::value_ptr(view));
			shareUBO.update(sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
			shareUBO.bindBase(0);

			const GLint modelLocation = program.getUniformLocation("model");
			const GLint textureLocation = program.getUniformLocation("ourTexture");
			glUniform1i(textureLocation, 0);
			glActiveTexture(GL_TEXTURE0);

			auto unwrap = []<typename Value>(Value&& value) -> decltype(auto)
			{
				using ValueType = std::remove_cvref_t<Value>;
				if constexpr (!std::is_same_v<ValueType, std::unwrap_reference_t<ValueType>>)
					return value.get();
				else
					return std::forward<Value>(value);
			};

			auto sprite = std::ranges::begin(sprites);
			auto physic = std::ranges::begin(physics);
			for (; sprite != std::ranges::end(sprites) && physic != std::ranges::end(physics); ++sprite, ++physic)
			{
				const auto& spriteComponent = unwrap(*sprite);
				const auto& physicsComponent = unwrap(*physic);
				glm::mat4 model(1.0f);
				model = glm::translate(model, physicsComponent.position);
				model = glm::rotate(model, glm::radians(spriteComponent.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(spriteComponent.size, 1.0f));

				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glBindTexture(GL_TEXTURE_2D, spriteComponent.texture.getID());
				glBindVertexArray(spriteComponent.mesh.VAO_s.getID());
				glDrawElements(
					GL_TRIANGLES,
					static_cast<GLsizei>(spriteComponent.mesh.EBO_s.IndexCount),
					GL_UNSIGNED_INT,
					nullptr
				);
			}

			glBindVertexArray(0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		template<std::ranges::range SpriteRange, std::ranges::range PhysicsRange>
		void DrawSpriteArrayByTexture(
			SpriteRange&& sprites,
			PhysicsRange&& physics,
			const Program& program,
			gl::UniformBuffer& shareUBO,
			const glm::mat4& view,
			const glm::mat4& projection
		)
		{
			if constexpr (std::ranges::sized_range<SpriteRange> && std::ranges::sized_range<PhysicsRange>)
			{
				assert(std::ranges::size(sprites) == std::ranges::size(physics));
				assert(std::ranges::size(sprites) > 0);
			}
			assert(shareUBO.isValid());

			program.use();
			shareUBO.update(0, sizeof(glm::mat4), glm::value_ptr(view));
			shareUBO.update(sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
			shareUBO.bindBase(0);

			const GLint modelLocation = program.getUniformLocation("model");
			const GLint textureLocation = program.getUniformLocation("ourTexture");
			glUniform1i(textureLocation, 0);
			glActiveTexture(GL_TEXTURE0);

			//使得可以处理 std::reference_wrapper<T> 类型的 range 元素
			auto unwrap = []<typename Value>(Value && value) -> decltype(auto)
			{
				using ValueType = std::remove_cvref_t<Value>;
				if constexpr (!std::is_same_v<ValueType, std::unwrap_reference_t<ValueType>>)
				{
					return value.get();
				}
				else
				{
					return std::forward<Value>(value);
				}
			};

			struct SpritePhysicsPair
			{
				const core::ecs::SpriteComponent* sprite;
				const core::ecs::PhysicsComponent* physics;
			};
			std::unordered_map<GLuint, std::vector<SpritePhysicsPair>> textureGroups;
			std::vector<GLuint> textureOrder;
			auto sprite = std::ranges::begin(sprites);
			auto spriteEnd = std::ranges::end(sprites);
			auto physic = std::ranges::begin(physics);
			auto physicEnd = std::ranges::end(physics);
			for (; sprite != spriteEnd && physic != physicEnd; ++sprite, ++physic)
			{
				const auto& spriteComponent = unwrap(*sprite);
				const auto& physicsComponent = unwrap(*physic);
				const GLuint textureID = spriteComponent.texture.getID();

				auto [group, inserted] = textureGroups.try_emplace(textureID);
				if (inserted)
					textureOrder.push_back(textureID);
				group->second.push_back(SpritePhysicsPair{ &spriteComponent, &physicsComponent });
			}

			for (const GLuint textureID : textureOrder)
			{
				glBindTexture(GL_TEXTURE_2D, textureID);
				for (const auto& pair : textureGroups.at(textureID))
				{
					const auto& spriteComponent = *pair.sprite;
					const auto& physicsComponent = *pair.physics;

					glm::mat4 model(1.0f);
					model = glm::translate(model, physicsComponent.position);
					model = glm::rotate(model, glm::radians(spriteComponent.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
					model = glm::scale(model, glm::vec3(spriteComponent.size, 1.0f));

					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
					glBindVertexArray(spriteComponent.mesh.VAO_s.getID());
					glDrawElements(
						GL_TRIANGLES,
						static_cast<GLsizei>(spriteComponent.mesh.EBO_s.IndexCount),
						GL_UNSIGNED_INT,
						nullptr
					);
				}
			}

			glBindVertexArray(0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	};

}
