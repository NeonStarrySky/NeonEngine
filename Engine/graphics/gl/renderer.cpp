#include "renderer.h"

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
			glGetUniformLocation(program.getID(), "model"),
			1,
			GL_FALSE,
			glm::value_ptr(model)
		);

		glUniformMatrix4fv(
			glGetUniformLocation(program.getID(), "view"),
			1,
			GL_FALSE,
			glm::value_ptr(view)
		);

		glUniformMatrix4fv(
			glGetUniformLocation(program.getID(), "projection"),
			1,
			GL_FALSE,
			glm::value_ptr(projection)
		);


		// 绘制面
		glUniform4fv(
			glGetUniformLocation(program.getID(), "ourColor"),
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
			glGetUniformLocation(program.getID(), "ourColor"),
			1,
			glm::value_ptr(invertedColor)
		);


		glBindVertexArray(mesh.VAO_l.getID());

		glDrawElements(
			GL_LINES,
			mesh.EBO_l.IndexCount,
			GL_UNSIGNED_INT,
			nullptr
		);

		glBindVertexArray(0);


		glUseProgram(0);
	}
}