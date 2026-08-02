#pragma once

#include "core/logger.h"
#include "graphics/gl/GLResource.hpp"

#include <source_location>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <glad/glad.h>

namespace neon::graphics::gl
{
	class ShaderManager
	{
	public:

		static void init();

		ShaderManager(neon::core::Logger* logger) : logger(logger)
		{
			assert(logger != nullptr && "Logger pointer cannot be null");
		}

		Shader loadAndCompileShader(const std::string& filePath, GLenum shaderType);
		bool loadShaders(const std::vector<std::pair<std::string, GLenum>>& shaderInfos);

		static void cleanup();

		static const Shader& getShader(const std::string& name, const std::source_location& loc = std::source_location::current());
		void linkProgram(const std::string& shaderName);
		bool linkPrograms(const std::vector<std::string>& names);
		gl::Program buildShader();
	private:
		neon::core::Logger* logger;
		std::vector<GLuint> shadersToLink;
		static std::unordered_map<std::string, Shader> shaderPrograms;
	};
}