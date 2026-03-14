#pragma once

#include "graphics/gl/shader.h"

#include <source_location>
#include <source_location>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <glad.h>

namespace neon::graphics::gl
{
	class ShaderManager
	{
	public:

		static void init();

		Shader loadAndCompileShader(const std::string& filePath, GLenum shaderType);
		bool loadShaders(const std::vector<std::pair<std::string, GLenum>>& shaderInfos);

		static void cleanup();

		static const Shader& getShader(const std::string& name, const std::source_location& loc = std::source_location::current());
		void linkProgram(const std::string& shaderName);
		bool linkPrograms(const std::vector<std::string>& names);
		Shader buildShader();
	private:
		std::vector<Shader::HandleType> shadersToLink;
		static std::unordered_map<std::string, Shader> shaderPrograms;
	};
}