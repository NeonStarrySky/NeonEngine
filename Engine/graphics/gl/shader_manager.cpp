#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "graphics/gl/GLResource.hpp"
#include "graphics/gl/shader_manager.h"
#include "tool/conhost.h"


namespace neon::graphics::gl
{
	std::unordered_map<std::string, Shader> ShaderManager::shaderPrograms;

	void ShaderManager::init()
	{
		// Èñ╝ËÈÏÎ┼╔½ã¸│╠ð‗
	}

	Shader ShaderManager::loadAndCompileShader(const std::string& filePath, GLenum shaderType)
	{
#ifdef _DEBUG
		std::cout << "Star to load and compile shader <" << filePath << ">\n";
		std::cout << "Shader type: " << (shaderType == GL_VERTEX_SHADER ? "Vertex Shader" :
			shaderType == GL_FRAGMENT_SHADER ? "Fragment Shader" :
			shaderType == GL_GEOMETRY_SHADER ? "Geometry Shader" : "Unknown") << std::endl;
#endif // _DEBUG

		// 1. Â┴╚í╬─╝■
		std::ifstream file(filePath);
		if (!file.is_open())
		{
			throw std::runtime_error("Cannot open shader file: " + filePath);
		}

		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string source = buffer.str();

		// 2. ┤┤¢¿Î┼╔½ã¸ÂÈ¤¾ú¿Ë├ RAII ╣▄└Ýú®

		Shader shader(glCreateShader(shaderType), shaderType);

		const char* src = source.c_str();
		glShaderSource(shader.getID(), 1, &src, nullptr);
		glCompileShader(shader.getID());

		// 3. ╝ý▓Ú▒ÓÊÙÎ┤╠¼
		GLint success = 0;
		glGetShaderiv(shader.getID(), GL_COMPILE_STATUS, &success);
		if (!success)
		{
			GLint logLength = 0;
			glGetShaderiv(shader.getID(), GL_INFO_LOG_LENGTH, &logLength);
			std::string infoLog(logLength, ' ');
			glGetShaderInfoLog(shader.getID(), logLength, nullptr, infoLog.data());
			throw std::runtime_error("Shader compilation failed (" + filePath + "): " +
				infoLog);
		}

		return shader; // ÀÁ╗ÏÍÃ─▄Í©ıÙ
	}

	bool ShaderManager::loadShaders(const std::vector<std::pair<std::string, GLenum>>& shaderInfos)//┼·┴┐╝ËÈÏÎ┼╔½ã¸(╬─╝■┬À¥Â + └Óð═)
	{
		for (const auto& [filePath, shaderType] : shaderInfos)
		{
			try
			{
				if (shaderPrograms.contains(filePath))
				{
					std::cerr << "Warning: Shader already loaded: " << filePath << std::endl;
					return false; // ÊÐ¥¡╝ËÈÏ╣²┴╦ú¼ÀÁ╗Ï false
				}
				Shader shader = loadAndCompileShader(filePath, shaderType);
				shaderPrograms[filePath] = std::move(shader);

			}
			catch (const std::runtime_error& e)
			{
				std::cerr << e.what() << std::endl;
				return false; // ╩º░▄╩▒ÀÁ╗Ï false
			}
		}
		return true;
	}

	void ShaderManager::cleanup()
	{
		shaderPrograms.clear();
	}

	const Shader& ShaderManager::getShader(const std::string& name, const std::source_location& loc)
	{
		auto it = shaderPrograms.find(name);
		if (it == shaderPrograms.end())
		{
			std::string msg = "Shader program not found: " + name;
			throw std::runtime_error(
				msg +
				" | File: " + loc.file_name() +
				" | Line: " + std::to_string(loc.line())
			);
		}
		return it->second;
	}

	void ShaderManager::linkProgram(const std::string& shaderName)
	{

		auto temp = getShader(shaderName).getID();
		shadersToLink.push_back(temp);
	}

	bool ShaderManager::linkPrograms(const std::vector<std::string>& names)
	{
		for (const auto& i : names) {
			try {
				linkProgram(i);
			}
			catch (const std::runtime_error& e) {
				shadersToLink.clear();//┤╦┤╬┴┤¢Ë╩º░▄ú¼ÃÕ┐ı┤²┴┤¢Ë┴ð▒Ý
				std::cerr << e.what() << std::endl;
				return false;
			}
		}
		return true;
	}

	gl::Program ShaderManager::buildShader()
	{
		gl::Program program(glCreateProgram());
		for (auto& i : shadersToLink)
		{
			glAttachShader(program.getID(), i);
		}
		glLinkProgram(program.getID());

		// ╝ý▓Ú┴┤¢ËÎ┤╠¼
		GLint success = 0;
		glGetProgramiv(program.getID(), GL_LINK_STATUS, &success);
		if (!success)
		{
			GLint logLength = 0;
			glGetProgramiv(program.getID(), GL_INFO_LOG_LENGTH, &logLength);
			std::string infoLog(logLength > 0 ? logLength : 1, ' ');
			glGetProgramInfoLog(program.getID(), logLength, nullptr, infoLog.data());

			throw std::runtime_error("Program link failed: " + infoLog);
		}

		return program;
	}
}
