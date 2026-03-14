#include "renderer.h"

#include <glad.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
namespace neon::graphics::gl
{
	void Renderer::Draw(const Mesh& mesh, const Shader& shader, const glm::vec4& color, GLfloat t, const glm::vec3& cameraFront, const glm::vec3& cameraPosition, const glm::vec3& worldUp)
	{
		shader.Use();

		// ªÊ÷∆√Ê
		glBindVertexArray(mesh.VAO_s.getID());



		glUniform4fv(glGetUniformLocation(shader.getID(), "ourColor"),
			1, glm::value_ptr(color));//color

		glUniform3fv
		(
			glGetUniformLocation(shader.getID(), "cameraFront"),
			1,
			glm::value_ptr(cameraFront)
		);//

		glUniform3fv
		(
			glGetUniformLocation(shader.getID(), "cameraPosition"),
			1,
			glm::value_ptr(cameraPosition)
		);//

		/*glUniform3fv
		(
			glGetUniformLocation(shader.getID(), "worldUp"),
			1,
			glm::value_ptr(worldUp)
		);*/
		glDrawElements(GL_TRIANGLES, mesh.EBO_s.IndexCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);


		glUniform4fv(glGetUniformLocation(shader.getID(), "ourColor"),
			1, glm::value_ptr(
				glm::vec4(1.0f - color.r, 1.0f - color.g, 1.0f - color.b, color.a)
			));
		glBindVertexArray(mesh.VAO_l.getID());
		glDrawElements(GL_LINES, mesh.EBO_l.IndexCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}