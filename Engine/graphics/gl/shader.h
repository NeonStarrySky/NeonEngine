#pragma once


#ifdef _DEBUG
#include "tool/conhost.h"
#endif // _DEBUG

#include "window.h"
#include "GLResource.h"

#include <algorithm>
#include <glad.h>
#include <type_traits>
#include <iostream>


namespace neon::graphics::gl
{
	struct ShaderDeleter
	{
		void operator()(GLuint id)noexcept
		{

#ifdef _DEBUG
			if (id) {
				//tool::setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
				std::cout << "Shader with ID " << id << " deleted.\n\n";
				//tool::setColor();
			}
#endif // _DEBUG

			glDeleteShader(id);
		}
	};
	struct ProgramDeleter
	{
		void operator()(GLuint id)noexcept
		{
			glDeleteProgram(id);
#ifdef _DEBUG
			//tool::setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
			std::cout << "Program with ID " << id << " deleted.\n";
			//tool::setColor();
#endif // _DEBUG
		}
	};

	namespace shader_ {
		template<typename D>
		concept GLDeleter =
			std::is_nothrow_invocable_v<D, GLuint>;
	}
	template<shader_::GLDeleter Deleter>
	class Shader_
	{
		GLResource<Deleter> shaderResource; // 管理着色器资源的生命周期
		GLenum type; // 记录着色器类型（顶点、片段等）
	public:
		using HandleType = typename GLResource<Deleter>::HandleType;

		Shader_() : shaderResource(0), type(0)
		{}
		Shader_(GLint id, GLenum type) : shaderResource(id), type(type)
		{}

		bool isValid() const { return shaderResource.isValid(); }

		void Use() const
		{
			if (isValid())
			{
				glUseProgram(shaderResource.getID());
			}
		}

		GLuint getID() const
		{
			return shaderResource.getID();
		}
		GLenum getType() const
		{
			return type;
		}
		//移动语义
		Shader_(Shader_&& other) noexcept
		{
			shaderResource = std::move(other.shaderResource);
			type = other.type;
		}
		Shader_& operator=(Shader_&& other) noexcept
		{
			if (this != &other)
			{
				shaderResource = std::move(other.shaderResource);
			}
			return *this;
		}
		~Shader_()
		{

		}
		//拷贝语义没必要
		Shader_(const Shader_&) = delete;
		Shader_& operator=(const Shader_&) = delete;
	};
	using Shader = Shader_<ShaderDeleter>;
	using Program = Shader_<ProgramDeleter>;

}