#pragma once

#include <iostream>

#include <glad.h>
#include <type_traits>

namespace neon::graphics::gl
{
	namespace GLResource_ {
		template<typename D>
		concept GLDeleter =
			std::is_nothrow_invocable_v<D, GLuint>;
	}


	template<GLResource_::GLDeleter Deleter>
	class GLResource
	{
		GLuint id = 0;
	public:

		using HandleType = typename GLuint;

		GLResource() :id(0) {};
		explicit GLResource(GLuint id) : id(id) {}

		bool isValid() const
		{
			return id != 0;
		}

		GLuint getID() const
		{
			return id;
		}
		// 提供一个获取 ID 的指针的方法，方便与 OpenGL 函数交互，请不要直接修改这个指针指向的值，除非你知道自己在做什么
		GLuint* getIDPtr()
		{
			return &id;
		}
		//移动语义
		GLResource(GLResource&& other) noexcept
		{
			id = other.id;
			other.id = 0;
		}
		GLResource& operator=(GLResource&& other) noexcept
		{
			if (this == &other)
			{
				return *this;
			}
			Deleter{}(id);
			id = other.id;
			other.id = 0;
			return *this;
		}
		GLuint release() noexcept
		{
			GLuint temp = id;
			id = 0;
			return temp;
		}
		//拷贝语义没必要
		GLResource(const GLResource&) = delete;
		GLResource& operator=(const GLResource&) = delete;

		~GLResource()
		{
			if (id)
			{
				Deleter{}(id);
			}
		}
	};
}