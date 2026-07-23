#pragma once

#include "GLResource.hpp"

#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <set>
#include <utility>
#include <vector>

namespace neon::graphics::gl
{
	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;

	};

	class Mesh {
	public:

		struct VAO_Deleter {
			void operator()(GLuint vao) const noexcept {
				glDeleteVertexArrays(1, &vao);
			}
		};
		struct EBO_Deleter {
			void operator()(GLuint ebo) const noexcept {
				glDeleteBuffers(1, &ebo);
			}
		};
		struct VBO_Deleter {
			void operator()(GLuint vbo) const noexcept {
				glDeleteBuffers(1, &vbo);
			}
		};

		struct EBO {
			GLResource<EBO_Deleter> EBO;
			unsigned int IndexCount;
		};

		GLResource<VAO_Deleter> VAO_s, VAO_l;
		GLResource<VBO_Deleter> VBO;
		EBO EBO_s, EBO_l;

		// 构造函数：仅负责数据上传
		Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

		// 析构函数：仅负责资源销毁
		~Mesh() {};

		// 禁用拷贝构造，防止重复删除 GPU 缓冲（或使用智能指针管理）
		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;

		static std::vector<unsigned int> makeWireframeEBO_unique(const std::vector<unsigned int>& indices);
	};
}