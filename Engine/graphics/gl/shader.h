#pragma once


#ifdef _DEBUG
#include "conhost.h"
#endif // _DEBUG

#include "graphics/gl/window.h"
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
				//setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
				std::cout << "Shader with ID " << id << " deleted.\n\n";
				//setColor();
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
			//setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
			std::cout << "Program with ID " << id << " deleted.\n";
			//setColor();
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

#ifdef _DEBUG
		void msg(const std::string& extra = "...")
		{
			setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			std::cout
				<< "\nClass Shader created!"
				<< "\ntype <" << GLenumToString(type) << "> "
				<< "\nid <" << shaderResource.getID() << "> "
				<< "\n" << extra << "\n\n";
			setColor();
		}
#endif
		Shader_() : shaderResource(0), type(0)
		{
			/*#ifdef _DEBUG
						setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
						std::cout << "A empty shader created, ID is 0, type is 0. This is likely a placeholder or an uninitialized shader.\n";
						setColor();
			#endif // DEBUG*/
		}
		Shader_(GLint id, GLenum type) : shaderResource(id), type(type)
		{
#ifdef _DEBUG
			//std::cout << GLenumToString(type) << " shader created by Shader_(GLint id, GLenum type) with ID: " << id << ".\n";
			msg();
#endif // DEBUG
		}

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
#ifdef _DEBUG
			msg();
#endif // DEBUG
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