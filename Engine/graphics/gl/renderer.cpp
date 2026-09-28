#include "renderer.h"

#include "core/ecs/components/physics_component.h"

#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
namespace neon::graphics
{
	void Renderer::Draw(
		const Mesh& mesh,
		const Program& program,
		const glm::vec4& color,
		const glm::mat4& model,
		const glm::mat4& view,
		const glm::mat4& projection
	)
	{
		program.use();

		// 上传矩阵
		glUniformMatrix4fv(
			program.getUniformLocation("model"),
			1,
			GL_FALSE,
			glm::value_ptr(model)
		);

		glUniformMatrix4fv(
			program.getUniformLocation("view"),
			1,
			GL_FALSE,
			glm::value_ptr(view)
		);

		glUniformMatrix4fv(
			program.getUniformLocation("projection"),
			1,
			GL_FALSE,
			glm::value_ptr(projection)
		);


		// 绘制面
		glUniform4fv(
			program.getUniformLocation("ourColor"),
			1,
			glm::value_ptr(color)
		);

		glBindVertexArray(mesh.VAO_s.getID());

		glDrawElements(
			GL_TRIANGLES,
			mesh.EBO_s.IndexCount,
			GL_UNSIGNED_INT,
			nullptr
		);

		glBindVertexArray(0);



		// 绘制线框

		glm::vec4 invertedColor(
			1.0f - color.r,
			1.0f - color.g,
			1.0f - color.b,
			color.a
		);


		glUniform4fv(
			program.getUniformLocation("ourColor"),
			1,
			glm::value_ptr(invertedColor)
		);


		glBindVertexArray(mesh.VAO_s.getID());

		glDrawElements(
			GL_LINES,
			mesh.EBO_l.IndexCount,
			GL_UNSIGNED_INT,
			nullptr
		);

		glBindVertexArray(0);


		glUseProgram(0);
	}

	void Renderer::Draw(
		const core::ecs::SpriteComponent& sprite,
		const core::ecs::PhysicsComponent& physic,
		const Program& program,
		const glm::mat4& view,
		const glm::mat4& projection
	) {
		program.use();

		// 1. 构建模型矩阵
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, physic.position);
		model = glm::rotate(model, glm::radians(sprite.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(sprite.size, 1.0f));

		// 2. 获取 uniform 位置并设置（Program 会缓存查询结果）
		GLint locModel = program.getUniformLocation("model");
		glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));

		GLint locView = program.getUniformLocation("view");
		glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));

		GLint locProj = program.getUniformLocation("projection");
		glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(projection));

		// 3. 纹理
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sprite.texture.getID());

		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER,
			GL_LINEAR
		);

		GLint locTex = program.getUniformLocation("ourTexture");
		glUniform1i(locTex, 0);

		// 4. 绘制
		glBindVertexArray(sprite.mesh.VAO_s.getID());
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// 5. 解绑
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}
