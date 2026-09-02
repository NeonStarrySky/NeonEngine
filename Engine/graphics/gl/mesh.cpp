#include "mesh.h"
#include <set>
#include <utility>
#include <vector>

namespace neon::graphics::gl
{
	// 构造函数：上传顶点数据和索引数据，分别为面和线框创建 VAO 和 EBO
	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
	{
		// 初始化面的 EBO 结构体
		EBO_s.IndexCount = static_cast<unsigned int>(indices.size());

		glGenVertexArrays(1, VAO_s.getIDPtr());
		glGenBuffers(1, VBO.getIDPtr());
		glGenBuffers(1, EBO_s.EBO.getIDPtr());

		glBindVertexArray(VAO_s.getID());

		glBindBuffer(GL_ARRAY_BUFFER, VBO.getID());
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_s.EBO.getID());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		// 顶点属性分配 (Layout)
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

		glBindVertexArray(0);

		// 线框的 EBO 结构体
		auto wireframeIndices = makeWireframeEBO_unique(indices);
		EBO_l.IndexCount = static_cast<unsigned int>(wireframeIndices.size());

		glGenVertexArrays(1, VAO_line.getIDPtr());
		glGenBuffers(1, VBO.getIDPtr());
		glGenBuffers(1, EBO_l.EBO.getIDPtr());

		glBindVertexArray(VAO_line.getID());

		glBindBuffer(GL_ARRAY_BUFFER, VBO.getID());
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_l.EBO.getID());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

		glBindVertexArray(0);
	}

	// 生成mesh的线框 EBO，确保每条边只出现一次
	std::vector<unsigned int> Mesh::makeWireframeEBO_unique(const std::vector<unsigned int>& indices)
	{
		std::set<std::pair<unsigned int, unsigned int>> edges;

		auto addEdge = [&](unsigned int a, unsigned int b)
			{
				if (a > b) std::swap(a, b);
				edges.emplace(a, b);
			};

		for (size_t i = 0; i + 2 < indices.size(); i += 3)
		{
			unsigned int a = indices[i];
			unsigned int b = indices[i + 1];
			unsigned int c = indices[i + 2];

			addEdge(a, b);
			addEdge(b, c);
			addEdge(c, a);
		}

		std::vector<unsigned int> wire;
		wire.reserve(edges.size() * 2);

		for (auto& e : edges)
		{
			wire.push_back(e.first);
			wire.push_back(e.second);
		}

		return wire;
	}

}